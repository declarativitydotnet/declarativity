package org.apache.hadoop.bufmanager;

import java.io.IOException;

import org.apache.hadoop.mapred.OutputCollector;

public interface MapOutputCollector<K extends Object, V extends Object> 
extends OutputCollector<K, V> {
    public void close() throws IOException;
    
    public void flush() throws IOException;

}
