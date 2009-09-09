package org.apache.hadoop.mapred.bufmanager;

import java.io.IOException;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.mapred.OutputCollector;

public interface JBufferCollector<K extends Object, V extends Object>
		extends OutputCollector<K, V> {
	
	public boolean reserve(long bytes);
	
	public void unreserve(long bytes);
	
	public void collect(DataInputBuffer key, DataInputBuffer value) throws IOException;
	
	public void spill(Path data, long dataSize, Path index) throws IOException;
	
	public void close();
}
