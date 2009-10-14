package org.apache.hadoop.mapred.bufmanager;

import java.io.IOException;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.util.Progress;

public interface JBufferCollector<K extends Object, V extends Object>
		extends OutputCollector<K, V> {
	
	public boolean reserve(long bytes);
	
	public void unreserve(long bytes);
	
	public void collect(DataInputBuffer key, DataInputBuffer value) throws IOException;
	
	public enum SpillOp {NOOP, COPY, RENAME};

	/**
	 * Spill the data and index files directly to the buffer. 
	 * @param data The data file.
	 * @param index The index file.
	 * @param op The operation to perform.
	 * @throws IOException
	 */
	public void spill(Path data, Path index, SpillOp op) throws IOException;
	
	public OutputFile close() throws IOException;
	
	public void reset(boolean restart) throws IOException;
	
	public Progress getProgress();
}
