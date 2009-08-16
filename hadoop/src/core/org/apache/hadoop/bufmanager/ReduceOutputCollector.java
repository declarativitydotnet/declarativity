package org.apache.hadoop.bufmanager;

import java.io.IOException;

import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.mapred.OutputCollector;

/*
 * This is an optimized version of the OutputCollector interface used for reduce
 * output. The difference is that the arguments to collect are still in their
 * marshalled representation.
 */
public interface ReduceOutputCollector<K extends Object, V extends Object>
		extends OutputCollector<K, V> {
	void collect(DataInputBuffer key, DataInputBuffer value) throws IOException;
}
