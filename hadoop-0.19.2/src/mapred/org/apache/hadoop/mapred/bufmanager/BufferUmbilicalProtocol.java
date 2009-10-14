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
	
	public void output(OutputFile buffer) throws IOException;
}
