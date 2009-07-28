/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.hadoop.mapred;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.net.URI;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLConnection;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.bufmanager.Buffer;
import org.apache.hadoop.bufmanager.BufferID;
import org.apache.hadoop.bufmanager.BufferUmbilicalProtocol;
import org.apache.hadoop.bufmanager.RecordGroup;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.ChecksumFileSystem;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.LocalFileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.DataOutputBuffer;
import org.apache.hadoop.io.IOUtils;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableFactories;
import org.apache.hadoop.io.WritableFactory;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.Decompressor;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.mapred.IFile.*;
import org.apache.hadoop.mapred.Merger.Segment;
import org.apache.hadoop.metrics.MetricsContext;
import org.apache.hadoop.metrics.MetricsRecord;
import org.apache.hadoop.metrics.MetricsUtil;
import org.apache.hadoop.metrics.Updater;
import org.apache.hadoop.util.Progress;
import org.apache.hadoop.util.Progressable;
import org.apache.hadoop.util.ReflectionUtils;
import org.apache.hadoop.util.StringUtils;

/** A Reduce task. */
public class ReduceTask extends Task {

	private class MapOutputFetcher extends Thread {

		private TaskUmbilicalProtocol trackerUmbilical;

		private BufferUmbilicalProtocol bufferUmbilical;

		private BufferID bufid;

		public MapOutputFetcher(TaskUmbilicalProtocol trackerUmbilical, BufferUmbilicalProtocol bufferUmbilical, BufferID bufid) {
			this.trackerUmbilical = trackerUmbilical;
			this.bufferUmbilical = bufferUmbilical;
			this.bufid = bufid;
		}

		public void run() {
			Set<TaskID> finishedMapTasks = new HashSet<TaskID>();
			Set<TaskAttemptID>  mapTasks = new HashSet<TaskAttemptID>();

			int eid = 0;
			while (finishedMapTasks.size() < getNumMaps()) {
				try {
					TaskScheduleEvent events[] = 
						trackerUmbilical.getMapScheduleEvents(getJobID(), eid, Integer.MAX_VALUE);

					eid += events.length;

					// Process the TaskCompletionEvents:
					// 1. Save the SUCCEEDED maps in knownOutputs to fetch the outputs.
					// 2. Save the OBSOLETE/FAILED/KILLED maps in obsoleteOutputs to stop fetching
					//    from those maps.
					// 3. Remove TIPFAILED maps from neededOutputs since we don't need their
					//    outputs at all.
					for (TaskScheduleEvent event : events) {
						switch (event.getTaskStatus()) {
						case FAILED:
						case KILLED:
						case OBSOLETE:
						case TIPFAILED:
						{
							TaskAttemptID mapTaskId = event.getTaskAttemptId();
							if (!mapTasks.contains(mapTaskId)) {
								bufferUmbilical.cancelFetch(bufid, mapTaskId);
								mapTasks.remove(mapTaskId);
							}
						}
						break;
						case SUCCEEDED:
						{
							TaskAttemptID mapTaskId = event.getTaskAttemptId();
							finishedMapTasks.add(mapTaskId.getTaskID());
						}
						break;
						case RUNNING:
						{
							URI u = URI.create(event.getTaskTrackerHttp());
							String host = u.getHost();
							TaskAttemptID mapTaskId = event.getTaskAttemptId();
							if (!mapTasks.contains(mapTaskId)) {
								bufferUmbilical.fetch(bufid, mapTaskId, host);
								mapTasks.add(mapTaskId);
							}
						}
						break;
						}
					}
				}
				catch (IOException e) {
					e.printStackTrace();
				}

				try {
					Thread.sleep(100);
				} catch (InterruptedException e) { }
			}
		}
	}

	static {                                        // register a ctor
		WritableFactories.setFactory
		(ReduceTask.class,
				new WritableFactory() {
			public Writable newInstance() { return new ReduceTask(); }
		});
	}

	private static final Log LOG = LogFactory.getLog(ReduceTask.class.getName());
	private int numMaps;
	private CompressionCodec codec;


	{ 
		getProgress().setStatus("reduce"); 
		setPhase(TaskStatus.Phase.SHUFFLE);        // phase to start with 
	}

	private Progress copyPhase = getProgress().addPhase("copy");
	private Progress sortPhase  = getProgress().addPhase("sort");
	private Progress reducePhase = getProgress().addPhase("reduce");
	private Counters.Counter reduceInputKeyCounter = 
		getCounters().findCounter(Counter.REDUCE_INPUT_GROUPS);
	private Counters.Counter reduceInputValueCounter = 
		getCounters().findCounter(Counter.REDUCE_INPUT_RECORDS);
	private Counters.Counter reduceOutputCounter = 
		getCounters().findCounter(Counter.REDUCE_OUTPUT_RECORDS);
	private Counters.Counter reduceCombineInputCounter =
		getCounters().findCounter(Counter.COMBINE_INPUT_RECORDS);
	private Counters.Counter reduceCombineOutputCounter =
		getCounters().findCounter(Counter.COMBINE_OUTPUT_RECORDS);

	public ReduceTask() {
		super();
	}

	public ReduceTask(String jobFile, TaskAttemptID taskId,
			int partition, int numMaps) {
		super(jobFile, taskId, partition);
		this.numMaps = numMaps;
	}

	@Override
	public TaskRunner createRunner(TaskTracker tracker) throws IOException {
		return new ReduceTaskRunner(this, tracker, this.conf);
	}

	@Override
	public boolean isMapTask() {
		return false;
	}

	public int getNumMaps() { return numMaps; }

	/**
	 * Localize the given JobConf to be specific for this task.
	 */
	@Override
	public void localizeConfiguration(JobConf conf) throws IOException {
		super.localizeConfiguration(conf);
		conf.setNumMapTasks(numMaps);
	}

	private void writeObject(ObjectOutputStream out) throws IOException {
		write(out);
	}

	private void readObject(ObjectInputStream in) 
	throws IOException, ClassNotFoundException {
		readFields(in);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		super.write(out);

		out.writeInt(numMaps);                        // write the number of maps
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		super.readFields(in);

		numMaps = in.readInt();
	}


	@Override
	@SuppressWarnings("unchecked")
	public void run(JobConf job, final TaskUmbilicalProtocol umbilical, final BufferUmbilicalProtocol bufferUmbilical)
	throws IOException {
		Reducer reducer = (Reducer)ReflectionUtils.newInstance(job.getReducerClass(), job);

		// start thread that will handle communication with parent
		startCommunicationThread(umbilical);

		final Reporter reporter = getReporter(umbilical); 

		BufferID bufid = bufferUmbilical.create(getTaskID(), getPartition(), getJobFile(), Buffer.BufferType.GROUPED);
		MapOutputFetcher fetcher = new MapOutputFetcher(umbilical, bufferUmbilical, bufid);
		fetcher.start();

		try {
			fetcher.join();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		/* This will not close (block) until all fetches finish! */
		bufferUmbilical.close(bufid);

		setPhase(TaskStatus.Phase.REDUCE); 

		// make output collector
		String finalName = getOutputName(getPartition());

		FileSystem fs = FileSystem.get(job);

		final RecordWriter out = 
			job.getOutputFormat().getRecordWriter(fs, job, finalName, reporter);  

		OutputCollector collector = new OutputCollector() {
			@SuppressWarnings("unchecked")
			public void collect(Object key, Object value)
			throws IOException {
				out.write(key, value);
				reduceOutputCounter.increment(1);
				// indicate that progress update needs to be sent
				reporter.progress();
			}
		};
		
		// apply reduce function
		try {
			System.err.println("GET RECORDS");
			RecordGroup record = null;
			while ((record = (RecordGroup) bufferUmbilical.getNextRecord(bufid, getTaskID())) != null) {
				System.err.println("GOT RECORD");
				record.unmarshall(this.conf);
				System.err.println("READ RECORD: " + record);
				reduceInputKeyCounter.increment(1);
				reducer.reduce(record.key, record.values.iterator(), collector, reporter);
			}
			System.err.println("DONE");
		} catch (IOException ioe) {
			ioe.printStackTrace();
			throw ioe;
		} catch (Throwable t) {
			t.printStackTrace();
		}
		finally {
			//Clean up: repeated in catch block below
			try {
				reducer.close();
				out.close(reporter);
			} catch (IOException ignored) {}
		}
		done(umbilical);
	}
}
