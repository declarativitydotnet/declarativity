package org.apache.hadoop.mapred;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import org.apache.hadoop.io.RawComparator;

public class ReduceRecordMap implements Iterable<ReduceRecordMap.Record> {
	public class Record {
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
	}
	
	private RawComparator keyComparator;
	
	private HashMap<Record, Record> records;
	
	public ReduceRecordMap(RawComparator keyComparator) {
		this.keyComparator = keyComparator;
		this.records = new HashMap<Record, Record>();
	}
	
	public void add(Object key, Object value) {
		Record rec = new Record(this.keyComparator, key);
		if (this.records.containsKey(rec)) {
			this.records.get(rec).append(value);
		}
		else {
			rec.append(value);
			this.records.put(rec, rec);
		}
	}
	
	public Iterator<Record> iterator() {
		return this.records.values().iterator();
	}
}
