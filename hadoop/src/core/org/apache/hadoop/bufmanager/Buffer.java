package org.apache.hadoop.bufmanager;

import java.io.DataOutput;
import java.io.IOException;
import java.util.Iterator;

import org.apache.hadoop.mapred.JobConf;

public interface Buffer<K extends Object, V extends Object> {
	public static enum BufferType {UNSORTED, SORTED, GROUPED};

	public BufferID bufid();
	
	public Iterator<Record<K, V>> iterator() throws IOException;
	
	public void fetch(BufferID bufid, String source) throws IOException;
	
	public void done(BufferRequest request);
	
	public void done(BufferReceiver receiver);
	
	public void add(Record record) throws IOException;
	
	public void close();
	
}
