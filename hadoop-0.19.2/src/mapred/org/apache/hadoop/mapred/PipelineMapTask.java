package org.apache.hadoop.mapred;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.URI;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.SerializationFactory;
import org.apache.hadoop.mapred.TaskCompletionEvent.Status;
import org.apache.hadoop.mapred.bufmanager.BufferRequest;
import org.apache.hadoop.mapred.bufmanager.BufferUmbilicalProtocol;
import org.apache.hadoop.mapred.bufmanager.JBuffer;
import org.apache.hadoop.mapred.bufmanager.JBufferCollector;
import org.apache.hadoop.mapred.bufmanager.JBufferSink;
import org.apache.hadoop.util.ReflectionUtils;

public class PipelineMapTask extends MapTask implements JBufferCollector {
	  private static final Log LOG = LogFactory.getLog(PipelineMapTask.class.getName());
	
	
	private class ReduceOutputFetcher extends Thread {
		private TaskID reduceTaskId;

		private TaskUmbilicalProtocol trackerUmbilical;
		
		private BufferUmbilicalProtocol bufferUmbilical;

		private JBufferSink sink;
		
		public ReduceOutputFetcher(TaskUmbilicalProtocol trackerUmbilical, BufferUmbilicalProtocol bufferUmbilical, JBufferSink sink, TaskID reduceTaskId) {
			this.trackerUmbilical = trackerUmbilical;
			this.bufferUmbilical = bufferUmbilical;
			this.sink = sink;
			this.reduceTaskId = reduceTaskId;
		}

		public void run() {
			boolean requestSent = false;
			int eid = 0;
			while (true) {
				try {
					ReduceTaskCompletionEventsUpdate updates = 
						trackerUmbilical.getReduceCompletionEvents(getJobID(), eid, Integer.MAX_VALUE);

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
							return;
						case SUCCEEDED:
							if (requestSent) return;
						case RUNNING:
						{
							URI u = URI.create(event.getTaskTrackerHttp());
							String host = u.getHost();
							TaskAttemptID reduceAttemptId = event.getTaskAttemptId();
							if (reduceAttemptId.getTaskID().equals(reduceTaskId) && !requestSent) {
								LOG.debug("Map " + getTaskID() + " sending buffer request to reducer " + reduceAttemptId);
								BufferRequest request = new BufferRequest(reduceAttemptId, 0, host, sink.getAddress());
								try {
									bufferUmbilical.request(request);
									requestSent = true;
									if (event.getTaskStatus() == Status.SUCCEEDED) return;
								} catch (IOException e) {
									LOG.warn("BufferUmbilical problem in taking request " + request + ". " + e);
								}
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
					Thread.sleep(1000);
				} catch (InterruptedException e) { }
			}
		}
	}
	
	
	private boolean open;
	
	private Mapper mapper;
	
	private Reporter reporter;
	  
	Deserializer keyDeserializer;
	
	Deserializer valDeserializer;
	
	private Object keyObject = null;
	
	private Object valObject = null;
	
	private boolean isSnapshotting = false;
	
	public PipelineMapTask() {
		super();
	}
	
	public PipelineMapTask(String jobFile, TaskAttemptID taskId, int partition) {
		super(jobFile, taskId, partition, "", new BytesWritable());
	}
	
	@Override
	public int getNumberOfInputs() { return 1; }
	
	@Override
	public boolean isPipeline() {
		return !(jobCleanup || jobSetup || taskCleanup);
	}
	
	@Override
	public boolean isSnapshotting() {
		return this.isSnapshotting;
	}
	
	public TaskID pipelineReduceTask(JobConf job) {
		JobID reduceJobId = JobID.forName(job.get("mapred.job.pipeline"));
		return new TaskID(reduceJobId, false, getTaskID().getTaskID().id);
	}
	
	@Override
	public void localizeConfiguration(JobConf conf) throws IOException {
		super.localizeConfiguration(conf);
	}
	
	@Override
	public void write(DataOutput out) throws IOException {
		super.write(out);
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		super.readFields(in);
	}

	@Override
	@SuppressWarnings("unchecked")
	public void run(final JobConf job, final TaskUmbilicalProtocol umbilical, final BufferUmbilicalProtocol bufferUmbilical)
	throws IOException {
		reporter = getReporter(umbilical);
	    // start thread that will handle communication with parent
	    startCommunicationThread(umbilical);

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
	    
	    if (job.get("mapred.job.pipeline", null) == null) {
	    	throw new IOException("PipelineMapTask: mapred.job.pipeline is not defined!");
	    }
		setPhase(TaskStatus.Phase.PIPELINE); 

	    
	    Class keyClass = job.getInputKeyClass();
	    Class valClass = job.getInputValueClass();
	    SerializationFactory serializationFactory = new SerializationFactory(job);
	    keyDeserializer = serializationFactory.getDeserializer(keyClass);
	    valDeserializer = serializationFactory.getDeserializer(valClass);

	    
		int numReduceTasks = job.getNumReduceTasks();
		if (numReduceTasks > 0) {
			this.buffer = new JBuffer(bufferUmbilical, getTaskID(), job, reporter);
			if (job.getBoolean("mapred.map.tasks.pipeline.execution", false)) {
				this.buffer.pipeline(true);
			}
			this.buffer.setProgress(getProgress());
			collector = buffer;
		} else { 
			collector = new DirectMapOutputCollector(umbilical, job, reporter);
		}
		
	    this.mapper = ReflectionUtils.newInstance(job.getMapperClass(), job);

	    /* Kick off a sink for receiving the reducer output. */
		boolean snapshots = job.getBoolean("mapred.job.snapshots", false);
		JBufferSink sink  = new JBufferSink(job, getTaskID(), this, this, snapshots);
		sink.open();
		
		/* Start the reduce output fetcher */
		TaskID reduceTaskId = pipelineReduceTask(job);
		ReduceOutputFetcher rof = new ReduceOutputFetcher(umbilical, bufferUmbilical, sink, reduceTaskId);
		rof.setDaemon(true);
		
		synchronized (this) {
			open = true;
			rof.start();
			long begin = System.currentTimeMillis();
			while (!sink.complete()) {
				try { this.wait();
				} catch (InterruptedException e) { }
			}
		}
		
		setPhase(TaskStatus.Phase.MAP); 
		sink.close();                        // This will commit final snapshots, if necessary.
		collector.close();                   // Perform final flush
		bufferUmbilical.commit(getTaskID()); // Register final flush.

		done(umbilical);
	}
	
	@Override
	public boolean snapshots(List<JBufferSink.JBufferRun> runs, float progress) throws IOException {
		synchronized (this) {
			float maxProgress = conf.getFloat("mapred.snapshot.max.progress", 0.9f);
			if (progress > maxProgress) {
				return false; // done at this point.
			}
			else if (buffer.canSnapshot() == false) {
				return true;
			}
		
			isSnapshotting = true;
			try {
				System.err.println("PipelineMapTask: " + getTaskID() + " perform snapshot. progress = " + progress);
				for (JBufferSink.JBufferRun run : runs) {
					run.spill(this);
				}
				this.buffer.getProgress().set(progress);
				return this.buffer.snapshot();
			} finally {
				buffer.reset(true);
				isSnapshotting = false;
			}
		}
	}
	
	@Override
	public void collect(DataInputBuffer key, DataInputBuffer value) throws IOException {
		keyDeserializer.open(key);
		valDeserializer.open(value);
		keyObject = keyDeserializer.deserialize(keyObject);
		valObject = valDeserializer.deserialize(valObject);
		collect(keyObject, valObject);
	}

	@Override
	public boolean reserve(long bytes) {
		return true;
	}

	@Override
	public int spill(Path data, Path index, boolean copy) throws IOException {
		CompressionCodec codec = null;
		if (conf.getCompressMapOutput()) {
			Class<? extends CompressionCodec> codecClass =
				conf.getMapOutputCompressorClass(DefaultCodec.class);
			codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
		}
		FileSystem fs = FileSystem.getLocal(conf);
		IFile.Reader reader = new IFile.Reader(conf, fs, data, codec);
		DataInputBuffer key = new DataInputBuffer();
		DataInputBuffer value = new DataInputBuffer();
		while (reader.next(key, value)) {
			collect(key, value);
		}
		reader.close();
		return -1;
	}

	@Override
	public void unreserve(long bytes) {
	}

	@Override
	public void collect(Object key, Object value) throws IOException {
		synchronized (this) {
			if (open) {
				mapper.map(key, value, collector, reporter);
			}
			else throw new IOException("Unable to collect because PipelineMapTask is closed!");
		}
	}

	@Override
	public void close() throws IOException {
	}

	@Override
	public void reset(boolean restart) throws IOException {
		throw new IOException("PipelineMapTask: reset not supported!");
		// if (buffer != null) buffer.reset(restart);
	}

}
