package org.apache.hadoop.bufmanager;

import java.io.IOException;
import java.util.Iterator;
import java.util.concurrent.Executor;

import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.mapred.JobConf;

public class JBufferGroup<K extends Object, V extends Object> 
       extends JBuffer<K, V> {

	public JBufferGroup(BufferUmbilicalProtocol umbilical, Executor executor, JobConf job, BufferID bufid) throws IOException {
		super(umbilical, executor, job, bufid, BufferType.SORTED);
	}
	
	@Override
	public Iterator<Record<K, V>> iterator() throws IOException {
		return new RecordGroupIterator(super.iterator(), this.job.getOutputKeyComparator());
	}
	
	private static class RecordGroupIterator<K extends Object, V extends Object> 
	        implements Iterator<RecordGroup<K, V>> {
		private Iterator<Record<K, V>> sortedIterator;
		
		private Record<K, V> currentRecord;
		
		private RawComparator<K> comparator;
		
		public RecordGroupIterator(Iterator<Record<K, V>> sortedIterator, RawComparator<K> comparator) {
			this.sortedIterator = sortedIterator;
			this.comparator     = comparator;
			this.currentRecord  = this.sortedIterator.hasNext() ? 
					              this.sortedIterator.next() : null;
		}

		@Override
		public boolean hasNext() {
			return this.currentRecord != null;
		}

		@Override
		public RecordGroup<K, V> next() {
			try {
				if (this.currentRecord != null) {
					RecordGroup<K, V> group = new RecordGroup<K, V>();
					group.collect(currentRecord.key, currentRecord.value);
					while (this.sortedIterator.hasNext()) {
						Record<K, V> record = this.sortedIterator.next();
						if (this.comparator.compare(group.key, record.key) == 0) {
							group.collect(record.key, record.value);
						}
						else {
							this.currentRecord = record;
							return group;
						}
					}
					this.currentRecord = null; // We're all done.
					return group;
				}
			} catch (IOException e) {
				return null;
			}
			return null;
		}

		@Override
		public void remove() {
			/* unsupported */
		}
	}

}
