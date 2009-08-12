package org.apache.hadoop.bufmanager;

import java.io.IOException;

import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.mapred.OutputCollector;

public interface ReduceOutputCollector<K extends Object, V extends Object>
		extends OutputCollector<K, V> {
	
	  void collect(DataInputBuffer key, DataInputBuffer value) throws IOException;
}
