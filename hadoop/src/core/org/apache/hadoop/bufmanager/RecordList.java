package org.apache.hadoop.bufmanager;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.util.IndexedSortable;
import org.apache.hadoop.util.IndexedSorter;
import org.apache.hadoop.util.QuickSort;
import org.apache.hadoop.util.ReflectionUtils;

public class RecordList<K extends Object, V extends Object> implements IndexedSortable, Writable {
	
	class RecordIterator implements Record.RecordIterator<K, V> {

		private int index;

		public RecordIterator() {
			this.index = 0;
		}

		@Override
		public boolean hasNext() {
			return index < records.size();
		}

		@Override
		public Record<K, V> peek() {
			return hasNext() ? records.get(index) : null;
		}

		@Override
		public Record<K, V> next() {
			return hasNext() ? records.get(index++) : null;
		}

		@Override
		public void remove() {
			records.remove(index);
		}
	}
	
	private JobConf conf;
	
	private List<Record<K, V>> records;
	
	private boolean sorted;
	
    protected RawComparator<K> comparator;
	
	public RecordList() {
		this.conf       = null;
		this.records    = new Vector<Record<K, V>>();
		this.sorted     = false;
		this.comparator = null;
	}
	
	public RecordList(JobConf conf) {
		this();
		this.conf = conf;
	}
	
	public RecordList(RecordList copy) {
		this.conf       = copy.conf;
		this.records    = new Vector<Record<K, V>>(copy.records);
		this.sorted     = copy.sorted;
		this.comparator = copy.comparator;
	}
	
	public void setConf(JobConf conf) {
		this.conf = conf;
	}
	
	public Record<K, V> peek() {
		return this.records.size() > 0 ? this.records.get(0) : null;
	}
	
	public Record<K, V> pop() {
		return this.records.size() > 0 ? this.records.remove(0) : null;
	}
	
	public void sort() {
		IndexedSorter sorter = (IndexedSorter)
		ReflectionUtils.newInstance(
				conf.getClass("map.sort.class", QuickSort.class), conf);
		this.comparator = conf.getOutputKeyComparator();
		
		sorter.sort(RecordList.this, 0, this.records.size(), null);
		this.sorted = true;
	}
	
	public boolean sorted() {
		return this.sorted;
	}
	
	@Override
	public int compare(int i, int j) {
		return comparator.compare(this.records.get(i).key, this.records.get(j).key);
	}

	@Override
	public void swap(int i, int j) {
		Collections.swap(this.records, i, j);
	}
	
	public Record.RecordIterator<K, V> iterator() {
		return new RecordIterator();
	}

	public void add(Record<K, V> record) throws IOException {
		this.records.add(record);
		this.sorted = false;
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		this.sorted = in.readBoolean();
		
		while (true) {
			Record record = new Record();
			record.readFields(in);
			if (record.isNull()) {
				return;
			}
			this.records.add(record);
		}
	}

	@Override
	public void write(DataOutput out) throws IOException {
		Iterator<Record<K, V>> iter = null;
		
		if (this.conf != null && conf.getCombinerClass() != null) {
			sort();
			iter = new CombineRecordIterator<K, V>(conf, iterator(), this.comparator);
		}
		else iter = iterator(); 

		out.writeBoolean(this.sorted);
		while (iter.hasNext()) {
			Record<K, V> record = iter.next();
			if (conf != null) record.marshall(conf);
			record.write(out);
		}
		Record.NULL_RECORD.write(out);
	}

}
