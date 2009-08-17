package org.apache.hadoop.mapred.bufmanager;

import java.io.IOException;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.ipc.VersionedProtocol;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;

public interface BufferUmbilicalProtocol extends VersionedProtocol {
	long versionID = 0;

	public void commit(TaskAttemptID taskid) throws IOException;
	
	public void request(BufferRequest request) throws IOException;
	
	public boolean pipe(TaskAttemptID taskid, int spillId, int numReduces) throws IOException;

}
