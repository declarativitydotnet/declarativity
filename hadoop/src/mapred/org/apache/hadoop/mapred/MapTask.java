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

import static org.apache.hadoop.mapred.Task.Counter.COMBINE_INPUT_RECORDS;
import static org.apache.hadoop.mapred.Task.Counter.COMBINE_OUTPUT_RECORDS;
import static org.apache.hadoop.mapred.Task.Counter.MAP_INPUT_BYTES;
import static org.apache.hadoop.mapred.Task.Counter.MAP_INPUT_RECORDS;
import static org.apache.hadoop.mapred.Task.Counter.MAP_OUTPUT_BYTES;
import static org.apache.hadoop.mapred.Task.Counter.MAP_OUTPUT_RECORDS;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.bufmanager.BufferRequest;
import org.apache.hadoop.bufmanager.BufferUmbilicalProtocol;
import org.apache.hadoop.bufmanager.JBuffer;
import org.apache.hadoop.bufmanager.MapOutputCollector;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.io.serializer.SerializationFactory;
import org.apache.hadoop.io.serializer.Serializer;
import org.apache.hadoop.mapred.IFile.Writer;
import org.apache.hadoop.mapred.IFile.Reader;
import org.apache.hadoop.mapred.Merger.Segment;
import org.apache.hadoop.util.IndexedSortable;
import org.apache.hadoop.util.IndexedSorter;
import org.apache.hadoop.util.Progress;
import org.apache.hadoop.util.QuickSort;
import org.apache.hadoop.util.ReflectionUtils;

/** A Map task. */
public final class MapTask extends Task {
	/**
	 * The size of each record in the index file for the map-outputs.
	 */
	public static final int MAP_OUTPUT_INDEX_RECORD_LENGTH = 24;

	public TrackedRecordReader recordReader = null;
	
	public MapOutputCollector collector = null;


	private BytesWritable split = new BytesWritable();
	private String splitClass;
	private InputSplit instantiatedSplit = null;
	private final static int APPROX_HEADER_LENGTH = 150;

	private Boolean pipeline;

	private static final Log LOG = LogFactory.getLog(MapTask.class.getName());

	{   // set phase for this task
		setPhase(TaskStatus.Phase.MAP); 
	}

	public MapTask() {
		super();
	}

	public MapTask(String jobFile, TaskAttemptID taskId, 
			int partition, String splitClass, BytesWritable split, 
			Boolean pipeline) throws IOException {
		super(jobFile, taskId, partition);
		this.splitClass = splitClass;
		this.split = split;
		this.pipeline = pipeline;
	}

	@Override
	public boolean isMapTask() {
		return true;
	}

	@Override
	public void localizeConfiguration(JobConf conf) throws IOException {
		super.localizeConfiguration(conf);
		Path localSplit = new Path(new Path(getJobFile()).getParent(), 
				"split.dta");
		LOG.debug("Writing local split to " + localSplit);
		DataOutputStream out = FileSystem.getLocal(conf).create(localSplit);
		Text.writeString(out, splitClass);
		split.write(out);
		out.close();
	}

	@Override
	public TaskRunner createRunner(TaskTracker tracker) {
		return new MapTaskRunner(this, tracker, this.conf);
	}

	private void writeObject(java.io.ObjectOutputStream out) throws IOException {
		write(out);
	}

	private void readObject(java.io.ObjectInputStream in) throws IOException, ClassNotFoundException {
		readFields(in);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		super.write(out);
		out.writeBoolean(pipeline);
		Text.writeString(out, splitClass);
		if (split != null) split.write(out);
		else throw new IOException("SPLIT IS NULL");
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		super.readFields(in);
		this.pipeline = in.readBoolean();
		splitClass = Text.readString(in);
		split.readFields(in);
	}

	@Override
	InputSplit getInputSplit() throws UnsupportedOperationException {
		return instantiatedSplit;
	}

	public RecordReader getRecordReader() {
		return this.recordReader;
	}

	/**
	 * This class wraps the user's record reader to update the counters and progress
	 * as records are read.
	 * @param <K>
	 * @param <V>
	 */
	class TrackedRecordReader<K, V> 
	implements RecordReader<K,V> {
		private RecordReader<K,V> rawIn;
		private Counters.Counter inputByteCounter;
		private Counters.Counter inputRecordCounter;

		TrackedRecordReader(RecordReader<K,V> raw, Counters counters) {
			rawIn = raw;
			inputRecordCounter = counters.findCounter(MAP_INPUT_RECORDS);
			inputByteCounter = counters.findCounter(MAP_INPUT_BYTES);
		}

		public K createKey() {
			return rawIn.createKey();
		}

		public V createValue() {
			return rawIn.createValue();
		}

		public synchronized boolean next(K key, V value)
		throws IOException {

			setProgress(getProgress());
			long beforePos = getPos();
			boolean ret = rawIn.next(key, value);
			if (ret) {
				inputRecordCounter.increment(1);
				inputByteCounter.increment(Math.abs(getPos() - beforePos));
			}
			return ret;
		}
		public long getPos() throws IOException { return rawIn.getPos(); }
		public void close() throws IOException { rawIn.close(); }
		public float getProgress() throws IOException {
			return rawIn.getProgress();
		}
	}

	@Override
	@SuppressWarnings("unchecked")
	public void run(final JobConf job, final TaskUmbilicalProtocol umbilical, final BufferUmbilicalProtocol bufferUmbilical)
	throws IOException {
		int numReduceTasks = conf.getNumReduceTasks();
		LOG.info("numReduceTasks: " + numReduceTasks);
		final Reporter reporter = getReporter(umbilical);

		if (numReduceTasks > 0) {
			collector = new JBuffer(bufferUmbilical, getTaskID(), job, reporter);
		} else { 
			collector = new DirectMapOutputCollector(umbilical, job, reporter);
		}

		Thread requestThread = new Thread() {
			public void run() {
				while (!isInterrupted()) {
					try {
						BufferRequest request = bufferUmbilical.getRequest(getTaskID());
						if (request != null) {
							collector.register(request);
						}
						else {
							sleep(100);
						}
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
		};
		requestThread.start();
		
		// start thread that will handle communication with parent
		startCommunicationThread(umbilical);

		// reinstantiate the split
		try {
			instantiatedSplit = (InputSplit) 
			ReflectionUtils.newInstance(job.getClassByName(splitClass), job);
		} catch (ClassNotFoundException exp) {
			IOException wrap = new IOException("Split class " + splitClass + 
			" not found");
			wrap.initCause(exp);
			throw wrap;
		}
		DataInputBuffer splitBuffer = new DataInputBuffer();
		splitBuffer.reset(split.get(), 0, split.getSize());
		instantiatedSplit.readFields(splitBuffer);

		// if it is a file split, we can give more details
		if (instantiatedSplit instanceof FileSplit) {
			FileSplit fileSplit = (FileSplit) instantiatedSplit;
			job.set("map.input.file", fileSplit.getPath().toString());
			job.setLong("map.input.start", fileSplit.getStart());
			job.setLong("map.input.length", fileSplit.getLength());
		}

		RecordReader rawIn =                  // open input
			job.getInputFormat().getRecordReader(instantiatedSplit, job, reporter);
		this.recordReader = new TrackedRecordReader(rawIn, getCounters());

		MapRunnable runner =
			(MapRunnable)ReflectionUtils.newInstance(job.getMapRunnerClass(), job);

		try {
			runner.run(this.recordReader, collector, reporter);      
			// collector.flush();
		} catch (IOException e) {
			e.printStackTrace();
			throw e;
		} finally {
			//close
			this.recordReader.close();                               // close input
			collector.close();
		}
		done(umbilical);
	}

	class DirectMapOutputCollector<K, V>
	implements MapOutputCollector<K, V> {

		private RecordWriter<K, V> out = null;

		private Reporter reporter = null;

		private final Counters.Counter mapOutputRecordCounter;

		@SuppressWarnings("unchecked")
		public DirectMapOutputCollector(TaskUmbilicalProtocol umbilical,
				JobConf job, Reporter reporter) throws IOException {
			this.reporter = reporter;
			String finalName = getOutputName(getPartition());
			FileSystem fs = FileSystem.get(job);

			out = job.getOutputFormat().getRecordWriter(fs, job, finalName, reporter);

			Counters counters = getCounters();
			mapOutputRecordCounter = counters.findCounter(MAP_OUTPUT_RECORDS);
		}

		public void close() throws IOException {
			if (this.out != null) {
				out.close(this.reporter);
			}

		}

		public void flush() throws IOException {
		}

		public void collect(K key, V value) throws IOException {
			reporter.progress();
			out.write(key, value);
			mapOutputRecordCounter.increment(1);
		}

		@Override
		public void register(BufferRequest request) throws IOException {
			// TODO Auto-generated method stub
			
		}

	}

}
