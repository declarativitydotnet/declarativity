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
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.URI;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableFactories;
import org.apache.hadoop.io.WritableFactory;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.mapred.bufmanager.BufferRequest;
import org.apache.hadoop.mapred.bufmanager.BufferUmbilicalProtocol;
import org.apache.hadoop.mapred.bufmanager.JBuffer;
import org.apache.hadoop.mapred.bufmanager.JBufferSink;
import org.apache.hadoop.mapred.bufmanager.JBufferCollector;
import org.apache.hadoop.mapred.bufmanager.ValuesIterator;
import org.apache.hadoop.util.Progress;
import org.apache.hadoop.util.ReflectionUtils;

/** A Reduce task. */
public class ReduceTask extends Task {

	private class MapOutputFetcher extends Thread {

		private TaskUmbilicalProtocol trackerUmbilical;
		
		private BufferUmbilicalProtocol bufferUmbilical;

		private JBufferSink sink = null;
		
		public MapOutputFetcher(TaskUmbilicalProtocol trackerUmbilical, BufferUmbilicalProtocol bufferUmbilical, JBufferSink sink) {
			this.trackerUmbilical = trackerUmbilical;
			this.bufferUmbilical = bufferUmbilical;
			this.sink = sink;
		}

		public void run() {
			Set<TaskID> finishedMapTasks = new HashSet<TaskID>();
			Set<TaskAttemptID>  mapTasks = new HashSet<TaskAttemptID>();

			int eid = 0;
			while (!isInterrupted() && finishedMapTasks.size() < getNumberOfInputs()) {
				try {
					MapTaskCompletionEventsUpdate updates = 
						trackerUmbilical.getMapCompletionEvents(getJobID(), eid, Integer.MAX_VALUE, ReduceTask.this.getTaskID());

					eid += updates.events.length;

					// Process the TaskCompletionEvents:
					// 1. Save the SUCCEEDED maps in knownOutputs to fetch the outputs.
					// 2. Save the OBSOLETE/FAILED/KILLED maps in obsoleteOutputs to stop fetching
					//    from those maps.
					// 3. Remove TIPFAILED maps from neededOutputs since we don't need their
					//    outputs at all.
					for (TaskCompletionEvent event : updates.events) {
						switch (event.getTaskStatus()) {
						case FAILED:
						case KILLED:
						case OBSOLETE:
						case TIPFAILED:
						{
							TaskAttemptID mapTaskId = event.getTaskAttemptId();
							if (!mapTasks.contains(mapTaskId)) {
								sink.cancel(mapTaskId);
								mapTasks.remove(mapTaskId);
							}
						}
						break;
						case SUCCEEDED:
						{
							TaskAttemptID mapTaskId = event.getTaskAttemptId();
							finishedMapTasks.add(mapTaskId.getTaskID());
						}
						case RUNNING:
						{
							URI u = URI.create(event.getTaskTrackerHttp());
							String host = u.getHost();
							TaskAttemptID mapTaskId = event.getTaskAttemptId();
							if (!mapTasks.contains(mapTaskId)) {
								bufferUmbilical.request(new BufferRequest(mapTaskId, getPartition(), host, sink.getAddress()));
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
					int waittime = mapTasks.size() == getNumberOfInputs() ? 60000 : 1000;
					Thread.sleep(waittime);
				} catch (InterruptedException e) { return; }
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
	protected int numMaps;
	protected CompressionCodec codec;
	protected JBuffer buffer = null;
	
	private boolean mapPipeline = false;
	private boolean reducePipeline = false;
	
	private boolean snapshots = false;
	private float   snapshotThreshold = 1f;
	private boolean isSnapshotting = false;
	

	@Override
	public boolean isSnapshotting() {
		return this.isSnapshotting;
	}


	{ 
		getProgress().setStatus("reduce"); 
		setPhase(TaskStatus.Phase.SHUFFLE);        // phase to start with 
	}

	protected Progress copyPhase = getProgress().addPhase("copy sort");
	protected Progress reducePhase = getProgress().addPhase("reduce");
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
	public TaskRunner createRunner(TaskTracker tracker, TaskTracker.TaskInProgress tip) throws IOException {
		return new ReduceTaskRunner(tip, tracker, this.conf);
	}

	@Override
	public boolean isMapTask() {
		return false;
	}
	
	@Override
	public boolean isPipeline() {
		if (!(jobCleanup || jobSetup || taskCleanup)) {
			return conf != null && 
				   conf.getBoolean("mapred.reduce.pipeline", false);
		}
		return false;
	}

	@Override
	public int getNumberOfInputs() { return numMaps; }

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
	public boolean snapshots(List<JBufferSink.JBufferRun> runs, float progress) throws IOException {
		if (reducePipeline && !buffer.canSnapshot()) {
			return true; // can't do it now but keep trying.
		}
		else if (progress > 0.9f) {
			return false; // turn off snapshots at the point.
		}
		
		synchronized (this) {
			this.isSnapshotting = true;
			try {
				OutputCollector collector = null;
				for (JBufferSink.JBufferRun run : runs) {
					run.spill(buffer);
				}
				return snapshot(false);
			} finally {
				isSnapshotting = false;
			}
		}
	}
	
	private boolean snapshot(boolean save) throws IOException {
		Path data = null;
		Path index = null;
		float currentProgress = buffer.getProgress().get();
		
		buffer.flush();
		if (save) {
			Path oldData = mapOutputFile.getOutputFile(getTaskID());
			Path oldIndex = mapOutputFile.getOutputIndexFile(getTaskID());
			data  = new Path(oldData.getParent(), getTaskID().toString() + "_snapshot.out");
			index = new Path(oldIndex.getParent(), getTaskID().toString() + "_snapshotindex.out");
			FileSystem localFs = FileSystem.getLocal(conf);
			localFs.copyFromLocalFile(oldData, data);
			localFs.copyFromLocalFile(oldIndex, index);
		}
		
		try {
			if (reducePipeline && buffer.canSnapshot()) {
				buffer.reset(true); // Restart for reduce output.
				reduce(buffer, null, null);
				buffer.getProgress().set(currentProgress);
				buffer.snapshot(); // Send reduce snapshot result
			}
			else if (!reducePipeline) {
				String snapshotName = getSnapshotOutputName(getPartition(), currentProgress);
				FileSystem fs = FileSystem.get(conf);
				final RecordWriter out = 
					conf.getOutputFormat().getRecordWriter(fs, conf, snapshotName, null);  
				OutputCollector collector = new OutputCollector() {
					@SuppressWarnings("unchecked")
					public void collect(Object key, Object value)
					throws IOException {
						out.write(key, value);
					}
				};
				buffer.flush();
				reduce(collector, null, null);
				out.close(null);
				System.err.println("ReduceTask: snapshot created. file " + snapshotName);
			}
			return true;
		} catch (IOException e) {
			e.printStackTrace();
			return true; 
		} finally {
			buffer.reset(true);
			if (save) {
				buffer.spill(data, index, false);
				buffer.setProgress(this.copyPhase);
			}
		}
	}
	
	
	@Override
	@SuppressWarnings("unchecked")
	public void run(JobConf job, final TaskUmbilicalProtocol umbilical, final BufferUmbilicalProtocol bufferUmbilical)
	throws IOException {
		// start thread that will handle communication with parent
		startCommunicationThread(umbilical);

		final Reporter reporter = getReporter(umbilical); 
		initialize(job, reporter);
		
		
	    // check if it is a cleanupJobTask
	    if (jobCleanup) {
	      runJobCleanupTask(umbilical);
	      return;
	    }
	    if (jobSetup) {
	      runJobSetupTask(umbilical);
	      return;
	    }
	    if (taskCleanup) {
	      runTaskCleanupTask(umbilical);
	      return;
	    }
	    
	    float [] weights = {0.75f, 0.25f};
	    getProgress().setWeight(weights);

		
		JBuffer buffer = new JBuffer(bufferUmbilical, getTaskID(), job, reporter);
		buffer.setProgress(copyPhase);
		
		mapPipeline      = job.getBoolean("mapred.map.pipeline", false);
		reducePipeline   = job.getBoolean("mapred.reduce.pipeline", false);
		snapshots        = job.getBoolean("mapred.job.snapshots", false);
		
		int snapshotInterval = job.getInt("mapred.snapshot.interval", -1);
		snapshotThreshold = snapshotInterval < 0 ? 1f : 1 / (float) snapshotInterval;
		JBufferSink sink = new JBufferSink(job, getTaskID(), (JBufferCollector) buffer, this, 
				                           snapshots && snapshotInterval < 0);
		sink.open();
		
		MapOutputFetcher fetcher = new MapOutputFetcher(umbilical, bufferUmbilical, sink);
		fetcher.setDaemon(true);
		fetcher.start();
		
		copy(buffer, sink);
		fetcher.interrupt();
		
		long begin = System.currentTimeMillis();
		try {
			buffer.setProgress(reducePhase);
			reduce(job, reporter, buffer);
		} finally {
			reducePhase.complete();
			if (reducePipeline) {
				System.err.println("Reduce: register buffer " + getTaskID());
				bufferUmbilical.commit(getTaskID());
			}
			buffer.free();
		}
		
		done(umbilical);
		System.err.println("Reduce time = " + (System.currentTimeMillis() - begin) + " ms.");
	}
	
	protected void copy(JBuffer buffer, JBufferSink sink) throws IOException {
		this.buffer = buffer;
		synchronized (this) {
			while(!sink.complete()) {
				if (buffer.getProgress().get() > snapshotThreshold) {
					snapshotThreshold *= 2.0f;
					isSnapshotting = true;
					try { snapshot(true);
					} finally {
						isSnapshotting = false;
					}
				}
				try { this.wait();
				} catch (InterruptedException e) { }
			}
		}
		buffer.flush();
		System.err.println("ReduceTask: copy phase complete.");
		copyPhase.complete();
	}
	
	private void reduce(OutputCollector collector, Reporter reporter, Progress progress) throws IOException {
		Reducer reducer = (Reducer)ReflectionUtils.newInstance(conf.getReducerClass(), conf);
		// apply reduce function
		try {
			ValuesIterator values = buffer.iterator();
			while (values.more()) {
				reducer.reduce(values.getKey(), values, collector, reporter);
				values.nextKey();
				
		        if (progress != null) progress.set(values.getProgress().get());
		        if (reporter != null) reporter.progress();
			}
		} catch (Throwable t) {
			t.printStackTrace();
		}
		finally {
			//Clean up: repeated in catch block below
			reducer.close();
		}
	}
	
	private void reduce(JobConf job, final Reporter reporter, JBuffer buffer) throws IOException {
		setPhase(TaskStatus.Phase.REDUCE); 
		
		if (reducePipeline) {
			buffer.reset(true);
			reduce(buffer, reporter, reducePhase);
			buffer.close();
		}
		else {
			// make output collector
			String finalName = getOutputName(getPartition());
			FileSystem fs = FileSystem.get(job);
			final RecordWriter out = job.getOutputFormat().getRecordWriter(fs, job, finalName, reporter);  
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
			System.err.println("ReduceTask: create final output file " + finalName);
			reduce(collector, reporter, reducePhase);
			out.close(reporter);
		}
	}
}
