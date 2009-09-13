package org.apache.hadoop.mapred;

import java.io.IOException;
import java.util.List;
import java.util.TreeSet;

import org.apache.hadoop.mapred.bufmanager.BufferRequest;
import org.apache.hadoop.mapred.bufmanager.BufferUmbilicalProtocol;
import org.apache.hadoop.mapred.bufmanager.JBuffer;
import org.apache.hadoop.mapred.bufmanager.JBufferSink;
import org.apache.hadoop.mapred.bufmanager.ValuesIterator;
import org.apache.hadoop.util.ReflectionUtils;

public class PipelineReduceTask extends ReduceTask {
	
	private BufferUmbilicalProtocol bufferUmbilical;
	
	public PipelineReduceTask() {
		super();
	}

	public PipelineReduceTask(String jobFile, TaskAttemptID taskId,
			int partition, int numMaps) {
		super(jobFile, taskId, partition, numMaps);
	}
	
	public boolean isPipeline() {
		return true;
	}
	
	@Override
	public void snapshots(List<JBufferSink.Snapshot> runs) throws IOException {
		for (JBufferSink.Snapshot snapshot : runs) {
			buffer.spill(snapshot.data(), snapshot.length(), snapshot.index());
		}
		
		Reducer reducer = (Reducer)ReflectionUtils.newInstance(conf.getReducerClass(), conf);
		// apply reduce function
		try {
			buffer.pipeline(false); // turn pipeline on
			ValuesIterator values = buffer.iterator();
			while (values.more()) {
				reducer.reduce(values.getKey(), values, buffer, null);
				values.nextKey();
			}
		} catch (Throwable t) {
			t.printStackTrace();
		}
		finally {
			//Clean up: repeated in catch block below
			reducer.close();
		}
		this.buffer.snapshot();
		this.buffer.reset();
	}
	
	@Override
	@SuppressWarnings("unchecked")
	public void run(JobConf job, final TaskUmbilicalProtocol umbilical, final BufferUmbilicalProtocol bufferUmbilical)
	throws IOException {
		this.bufferUmbilical = bufferUmbilical;
		super.run(job, umbilical, bufferUmbilical);
	}
	
	protected void copy(JBuffer buffer, JBufferSink sink) throws IOException {
		this.buffer = buffer;
		super.copy(buffer, sink);
	}
	
	@Override
	public void reduce(JobConf job, final Reporter reporter, JBuffer buffer) throws IOException {
		setPhase(TaskStatus.Phase.REDUCE); 
		
		Reducer reducer = (Reducer)ReflectionUtils.newInstance(job.getReducerClass(), job);
		// apply reduce function
		try {
			buffer.pipeline(true); // turn pipeline on
			ValuesIterator values = buffer.iterator();
			while (values.more()) {
				reducer.reduce(values.getKey(), values, buffer, reporter);
				values.nextKey();
				
		        reducePhase.set(values.getProgress().get());
				reporter.progress();
			}
		} catch (IOException ioe) {
			ioe.printStackTrace();
			throw ioe;
		} catch (Throwable t) {
			t.printStackTrace();
		}
		finally {
			//Clean up: repeated in catch block below
			reducer.close();
			buffer.close();
			TreeSet<BufferRequest> requests = buffer.requests();
			if (requests.size() > 0) {
				System.err.println("PipelineReduceTask: " + getTaskID() + " flushed final.");
				for (BufferRequest request : requests) {
					request.flushFinal();
				}
			}
			else {
				bufferUmbilical.commit(getTaskID());
			}
		}
	}
}
