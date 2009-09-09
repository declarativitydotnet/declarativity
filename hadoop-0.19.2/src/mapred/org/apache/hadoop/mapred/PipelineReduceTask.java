package org.apache.hadoop.mapred;

import java.io.IOException;

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
		
		buffer.flush();
		buffer.close();
		
		JBuffer output  = new JBuffer(bufferUmbilical, getTaskID(), job, reporter, buffer.buffer());
		Reducer reducer = (Reducer)ReflectionUtils.newInstance(job.getReducerClass(), job);
		// apply reduce function
		try {
			Class keyClass = job.getMapOutputKeyClass();
			Class valClass = job.getMapOutputValueClass();

			ValuesIterator values = buffer.iterator();
			while (values.more()) {
				reducer.reduce(values.getKey(), values, output, reporter);
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
			output.flush();
			output.close();
			bufferUmbilical.commit(getTaskID());
		}
	}
}
