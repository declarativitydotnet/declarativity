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
	 * Statistic on how well pipelining is keeping up with the production
	 * of a task's output.
	 * @param owner The task producing output.
	 * @return 
	 * @throws IOException
	 */
	public float pipestat(TaskAttemptID owner) throws IOException;
	
	public void output(OutputFile buffer) throws IOException;
}
