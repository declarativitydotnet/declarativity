package org.apache.hadoop.bufmanager;

import java.io.DataOutput;
import java.io.IOException;
import java.util.Iterator;

import org.apache.hadoop.mapred.JobConf;

public interface Buffer<K extends Object, V extends Object> {
	public static enum BufferType {UNSORTED, SORTED, GROUPED};

	public BufferType type();
	
	public JobConf conf();
	
	public BufferID bufid();
	
	public Iterator<Record<K, V>> iterator() throws IOException;
	
	public void add(Record record) throws IOException;
	
	public void close() throws IOException;
	
	public void commit() throws IOException;
	
	public long memory();

	public void flush() throws IOException;
	
}
