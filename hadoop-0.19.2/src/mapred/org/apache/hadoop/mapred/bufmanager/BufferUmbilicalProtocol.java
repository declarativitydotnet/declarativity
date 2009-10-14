package org.apache.hadoop.mapred.bufmanager;

import java.io.IOException;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.ipc.VersionedProtocol;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;

public interface BufferUmbilicalProtocol extends VersionedProtocol {
	long versionID = 0;
	
	public void request(MapBufferRequest request) throws IOException;
	
	public void request(ReduceBufferRequest request) throws IOException;
	
	/**
	 * How many outputs have we received from the given task.
	 * Note: count includes all intermediate outputs and 
	 * the compete final output.
	 * @param owner The taskid sending the output buffers.
	 * @throws IOException
	 */
	public int outputs(TaskAttemptID owner) throws IOException;
	
	public void output(OutputFile buffer) throws IOException;
}
