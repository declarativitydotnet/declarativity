package org.apache.hadoop.bufmanager;

import java.io.IOException;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.ipc.VersionedProtocol;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;

public interface BufferUmbilicalProtocol extends VersionedProtocol {
	long versionID = 0;

	/**
	 * Add a record to a given output buffer.
	 * @param bufid The map output buffer identifier.
	 * @param output Register the map output at the given path.
	 * @throws IOException
	 */
	public void register(BufferID bufid, String output) throws IOException;
	
	public void request(BufferRequest request) throws IOException;
	
	public BufferRequest getRequest(BufferID bufid) throws IOException;

}
