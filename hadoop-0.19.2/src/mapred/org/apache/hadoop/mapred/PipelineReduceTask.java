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
	public synchronized boolean snapshots(List<JBufferSink.Snapshot> runs, float progress) throws IOException {
		if (!buffer.canSnapshot()) {
			System.err.println("PipelineReduceTask: " + getTaskID() + " can't do snapshot yet.");
			return true;
		}
		
		for (JBufferSink.Snapshot snapshot : runs) {
			buffer.spill(snapshot.data(), snapshot.length(), snapshot.index(), true);
		}
		
		Reducer reducer = (Reducer)ReflectionUtils.newInstance(conf.getReducerClass(), conf);
		// apply reduce function
		try {
			buffer.flush();
			buffer.reset(true);
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
		buffer.getProgress().set(progress);
		System.err.println("PipelineReduceTask: " + getTaskID() + " snapshot. progress = " + progress);
		return buffer.snapshot(true);
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
			ValuesIterator values = buffer.iterator();
			buffer.reset(true);

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
			bufferUmbilical.commit(getTaskID());
		}
	}
}
