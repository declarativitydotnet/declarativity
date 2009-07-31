package org.apache.hadoop.bufmanager;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.util.ReflectionUtils;

public class CombineRecordIterator<K extends Object, V extends Object> implements Iterator<Record<K, V>> {

	private Record.RecordIterator<K, V> records;

	private RawComparator<K> comparator;

	private Reducer combiner;

	public CombineRecordIterator(JobConf job, Record.RecordIterator<K, V> records, RawComparator<K> comparator) {
		this.records = records;
		this.comparator = comparator;

		Class<? extends Reducer> combinerClass = job.getCombinerClass();
		combiner = (Reducer)ReflectionUtils.newInstance(combinerClass, job);
	}

	@Override
	public boolean hasNext() {
		return this.records.hasNext();
	}

	@Override
	public Record<K, V> next() {
		try {
			return combineNext();
		} catch (IOException e) {
			return null;
		}
	}

	@Override
	public void remove() {
		/* Not supported. */
	}

	private Record<K, V> combineNext() throws IOException {
		if (!this.records.hasNext()) return null;

		List<V> values = new ArrayList<V>();
		Record<K, V> next = this.records.next();
		values.add(next.value);

		Record<K, V> test = this.records.peek();
		while (this.records.hasNext() && this.comparator.compare(next.key, test.key) == 0) {
			values.add(test.value);
			this.records.remove();
			test = this.records.peek();
		}

		Record combined = new Record();
		combiner.reduce(next.key, values.iterator(), combined, null);
		return combined;
	}
}