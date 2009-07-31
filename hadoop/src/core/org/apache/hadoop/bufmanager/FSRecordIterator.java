package org.apache.hadoop.bufmanager;

import java.io.IOException;

import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.mapred.JobConf;

public class FSRecordIterator<K extends Object, V extends Object> implements Record.RecordIterator<K, V> {
	
	private JobConf conf;
	
	private FSDataInputStream in;
	
	private Record current;
	
	public FSRecordIterator(JobConf conf, FSDataInputStream in) {
		this.conf = conf;
		this.in = in;
		readNext();
	}

	@Override
	public boolean hasNext() {
		return this.current != null;
	}

	@Override
	public Record<K, V> peek() {
		return this.current;
	}

	@Override
	public Record<K, V> next() {
		Record tmp = this.current;
		readNext();
		return tmp;
	}
	
	private boolean readNext() {
		try {
			Record<K, V> record = new Record<K, V>();
			record.readFields(in);
			record.unmarshall(conf);
			this.current = record;
			return true;
		} catch (IOException e) {
			this.current = null;
			return false;
		}
	}

	@Override
	public void remove() {
	}
}
