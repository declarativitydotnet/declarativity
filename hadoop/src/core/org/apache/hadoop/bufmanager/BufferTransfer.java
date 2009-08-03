package org.apache.hadoop.bufmanager;

import java.io.IOException;

public interface BufferTransfer<K extends Object, V extends Object> extends Buffer<K, V> {
	
	public void transfer(BufferID requestID, String source) throws IOException;
	
	public void cancel(BufferID requestID) throws IOException;
	
	public void done(BufferRequest request);
	
	public void done(BufferReceiver receiver);

}
