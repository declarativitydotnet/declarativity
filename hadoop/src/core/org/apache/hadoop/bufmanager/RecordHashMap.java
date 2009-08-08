package org.apache.hadoop.bufmanager;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.util.ReflectionUtils;

public class RecordHashMap<K extends Object, V extends Object> 
implements OutputCollector<K, V>, Iterable<RecordHashMap.Record<K, V>> {
	public static class Record<K, V> implements OutputCollector<K, V>{
		private RawComparator<K> comparator;
		
		private K key;
		
		private List<V> values;
		
		public Record(RawComparator<K> comparator, K key) {
			this.comparator = comparator;
			this.key = key;
			this.values = new ArrayList<V>();
		}
		
		public int hashCode() {
			return this.key.hashCode();
		}
		
		public boolean equals(Object o) {
			if (o instanceof Record) {
				Record<K, V> other = (Record) o;
				return this.comparator.compare(this.key, other.key) == 0;
			}
			return false;
		}
		
		public Object key() {
			return this.key;
		}
		
		public Iterator<V> values() {
			return this.values.iterator();
		}

		public void add(V value) {
			this.values.add(value);
		}

		@Override
		public void collect(K key, V value) throws IOException {
			this.key = key;
			add(value);
		}
	}
	
	private JobConf job;
	
	private RawComparator<K> keyComparator;
	
	private Map<Record<K, V>, Record<K, V>> records;
	
	private final Reducer combiner;

	
	public RecordHashMap(JobConf job) {
		this.job = job;
		this.keyComparator = job.getOutputKeyComparator();

		this.records = new HashMap<Record<K, V>, Record<K, V>>();
		
		Class<? extends Reducer> combinerClass = job.getCombinerClass();
		if (combinerClass != null) {
			combiner = (Reducer)ReflectionUtils.newInstance(combinerClass, job);
		} else this.combiner = null;
	}
	
	@Override
	public void collect(K key, V value) {
		Record<K, V> rec = new Record<K, V>(this.keyComparator, key);
		if (this.records.containsKey(rec)) {
			Record<K, V> record = this.records.get(rec);
			record.add(value);
			if (combiner != null) {
				Record combined = combine(record);
				record.values = combined.values;
			}
		}
		else {
			rec.add(value);
			this.records.put(rec, rec);
		}
	}
	
	public Iterator<Record<K, V>> iterator() {
		return this.records.values().iterator();
	}
	
	
	private Record<K, V> combine(Record<K, V> record) {
		Record<K, V> combinedRecord = new Record<K, V>(record.comparator, record.key);
		try {
			combiner.reduce(record.key, record.values(), combinedRecord, null);
			return combinedRecord;
		} catch (IOException e) {
			return record;
		}
	}
}
