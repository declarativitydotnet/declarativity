package org.apache.hadoop.mapred;

import java.io.IOException;

public interface FlushedOutputCollector<K, V> extends OutputCollector<K, V> {

	void forceFlush() throws IOException;
}
