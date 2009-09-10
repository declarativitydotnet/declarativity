package org.apache.hadoop.mapred;

import java.io.IOException;
import java.util.TreeSet;

import org.apache.hadoop.mapred.bufmanager.BufferRequest;
import org.apache.hadoop.mapred.bufmanager.BufferUmbilicalProtocol;
import org.apache.hadoop.mapred.bufmanager.JBuffer;
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
	@SuppressWarnings("unchecked")
	public void run(JobConf job, final TaskUmbilicalProtocol umbilical, final BufferUmbilicalProtocol bufferUmbilical)
	throws IOException {
		this.bufferUmbilical = bufferUmbilical;
		super.run(job, umbilical, bufferUmbilical);
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
			
			buffer.free();
		}
	}
}
