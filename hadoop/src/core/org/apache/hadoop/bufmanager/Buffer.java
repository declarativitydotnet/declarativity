package org.apache.hadoop.bufmanager;

import java.io.IOException;
import java.util.Iterator;

import org.apache.hadoop.mapred.JobConf;

public interface Buffer<K extends Object, V extends Object> {
	public static enum BufferType {UNSORTED, SORTED, GROUPED};

	public BufferID bufid();
	
	
	public JobConf conf();
	
	/**
	 * Add record to the buffer.
	 * @param record Record to add.
	 * @throws IOException If I don't like you
	 */
	public void add(Record<K, V> record) throws IOException;
	
	/**
	 * Get an iterator over the records in this buffer.
	 * If the recards are to be sorted or grouped, then
	 * this call will block until all records have been
	 * received, which occurs when all open calls have
	 * been closed. 
	 * @return An iterator over the records in this buffer.
	 * @throws IOException If I don't feel like it.
	 */
	public Iterator<Record<K, V>> records() throws IOException;
	
}
