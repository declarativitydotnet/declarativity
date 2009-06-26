package org.apache.hadoop.mapred;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.util.ReflectionUtils;

public class ReduceRecordMap implements Iterable<ReduceRecordMap.Record> {
	public static class Record implements OutputCollector {
		private RawComparator comparator;
		
		private Object key;
		
		private List<Object> values;
		
		public Record(RawComparator comparator, Object key) {
			this.comparator = comparator;
			this.key = key;
			this.values = new ArrayList();
		}
		
		public int hashCode() {
			return this.key.hashCode();
		}
		
		public boolean equals(Object o) {
			if (o instanceof Record) {
				Record other = (Record) o;
				return this.comparator.compare(this.key, other.key) == 0;
			}
			return false;
		}
		
		public void append(Object value) {
			this.values.add(value);
		}
		
		public Object key() {
			return this.key;
		}
		
		public Iterator values() {
			return this.values.iterator();
		}

		@Override
		public void collect(Object key, Object value) throws IOException {
			this.key = key;
			append(value);
		}
	}
	
	private JobConf job;
	
	private RawComparator keyComparator;
	
	private HashMap<Record, Record> records;
	
	private final Reducer combiner;

	
	public ReduceRecordMap(JobConf job, RawComparator keyComparator) {
		this.job = job;
		this.keyComparator = keyComparator;
		this.records = new HashMap<Record, Record>();
		
		Class<? extends Reducer> combinerClass = job.getCombinerClass();
		if (combinerClass != null) {
			combiner = (Reducer)ReflectionUtils.newInstance(combinerClass, job);
		} else this.combiner = null;
	}
	
	public void add(Object key, Object value) {
		Record rec = new Record(this.keyComparator, key);
		if (this.records.containsKey(rec)) {
			Record record = this.records.get(rec);
			record.append(value);
			if (combiner != null) {
				Record combined = combine(record);
				record.values = combined.values;
			}
		}
		else {
			rec.append(value);
			this.records.put(rec, rec);
		}
	}
	
	public Iterator<Record> iterator() {
		return this.records.values().iterator();
	}
	
	
	private Record combine(Record record) {
		Record combinedRecord = new Record(record.comparator, record.key);
		try {
			combiner.reduce(record.key, record.values(), combinedRecord, null);
			return combinedRecord;
		} catch (IOException e) {
			return record;
		}
	}
}
