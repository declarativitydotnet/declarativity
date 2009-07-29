package org.apache.hadoop.bufmanager;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import org.apache.hadoop.bufmanager.BufferManager.BufferControl;
import org.apache.hadoop.bufmanager.Record.RecordQueue;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.CompressionOutputStream;
import org.apache.hadoop.io.compress.Compressor;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapOutputFile;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.util.IndexedSortable;
import org.apache.hadoop.util.IndexedSorter;
import org.apache.hadoop.util.QuickSort;
import org.apache.hadoop.util.ReflectionUtils;

public class JBuffer<K extends Object, V extends Object> 
    implements BufferManager.BufferControl<K, V>, IndexedSortable {
	
	protected final JobConf job;
	
	protected BufferID bufid;
	
    protected MapOutputFile mapOutputFile;
	
    protected List<RecordStream> streams;
    
	protected List<Record<K, V>> records;
	
    protected final RawComparator<K> comparator;
    
    protected final FileSystem localFs;
    
    protected final IndexedSorter sorter;

    protected List<Path> spillFiles;

    protected long curMemorySize;
    
    protected BufferType type;
    
    protected boolean closed;
    
    private int total;
    
	public JBuffer(JobConf job, BufferID bufid, BufferType type) throws IOException {
		reset();
		this.job = job;
		this.bufid = bufid;
		this.type = type;
		this.closed = false;
		
		this.streams = new LinkedList<RecordStream>();
		
	    this.localFs = FileSystem.getLocal(job);
		this.mapOutputFile = new MapOutputFile(bufid.taskid().getJobID());
		this.mapOutputFile.setConf(job);
		
	    this.sorter = (IndexedSorter)
	        ReflectionUtils.newInstance(
	            job.getClass("map.sort.class", QuickSort.class), job);
	    this.comparator = job.getOutputKeyComparator();
	    
	    this.spillFiles = new ArrayList<Path>();
	}
	
	public int hashCode() {
		return this.bufid.hashCode();
	}
	
	public boolean equals(Object o) {
		if (o instanceof JBuffer) {
			return this.bufid.equals(((JBuffer)o).bufid);
		}
		return false;
	}
	
	public BufferID bufid() {
		return this.bufid;
	}
	
	private void reset() {
		this.records = new ArrayList<Record<K, V>>();
		this.curMemorySize = 0;
		this.total = 0;
	}
	
	public JobConf conf() {
		return this.job;
	}
	
	public CompressionCodec codec() {
		if (this.job != null && job.getCompressMapOutput()) {
			Class<? extends CompressionCodec> codecClass =
				job.getMapOutputCompressorClass(DefaultCodec.class);
			return (CompressionCodec) ReflectionUtils.newInstance(codecClass, job);
		}
		return null;
	}
	
	public long memory() {
		return this.curMemorySize;
	}
	
	/**
	 * Add a record to this buffer.
	 * @param record The record to add
	 * @throws IOException 
	 */
	public void add(Record record) throws IOException {
		synchronized (this) {
			if (closed) {
				throw new IOException("Buffer is closed!");
			}
			
			this.total++;
			this.records.add(record);
			this.curMemorySize += record.size();
			
			for (RecordStream stream : streams) {
				stream.add(record);
			}
		}
	}
	
	/**
	 * Get an iterator to the records in this buffer.
	 * @param sorted Sorted order by key?
	 * @return Record iterator
	 * @throws IOException 
	 */
	public Iterator<Record<K, V>> records() throws IOException {
		synchronized (this) {
			while (this.type == BufferType.SORTED && !closed) {
				try { this.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			
			if (this.type == BufferType.SORTED && this.records.size() > 1) {
				/* Sort in memory copy of the buffer */
				sorter.sort(JBuffer.this, 0, this.records.size(), null);
			}
			
			List<Record.RecordQueue<K, V>> queues = new ArrayList<Record.RecordQueue<K, V>>();
			queues.add(new RecordIterator(new ArrayList<Record>(this.records)));
			for (Path file : this.spillFiles) {
				FSDataInputStream in = localFs.open(file);
				queues.add(new FSRecordIterator(this.job, in));
			}
			
			if (this.type == BufferType.SORTED) {
				return new RecordMerger(comparator, queues);
			}
			else {
				RecordStream stream = new RecordStream(queues);
				this.streams.add(stream);
				return stream;
			}
		}
	}
	
	
	@Override
	public int compare(int i, int j) {
		return comparator.compare(this.records.get(i).key, this.records.get(j).key);
	}

	@Override
	public void swap(int i, int j) {
		Collections.swap(this.records, i, j);
	}

	
	@Override
	public void free() {
		// TODO
	}
	
	@Override
	public void close() {
		synchronized (this) {
			System.err.println("BUFFER TOTAL RECORDS " + this.total);
			this.closed = true;
			for (RecordStream stream : streams) {
				stream.done(true);
			}
		}
		
	}
	
	@Override
	public void flush() throws IOException {
		synchronized (this) {
			sorter.sort(JBuffer.this, 0, this.records.size(), null);

			// create spill file
			Path filename = mapOutputFile.getSpillFileForWrite(this.bufid.taskid(), this.spillFiles.size(), curMemorySize);
			this.spillFiles.add(filename);

			FSDataOutputStream out = localFs.create(filename);
			// compression
			if (job.getCompressMapOutput()) {
				Class<? extends CompressionCodec> codecClass =
					job.getMapOutputCompressorClass(DefaultCodec.class);
				CompressionCodec codec = (CompressionCodec)
				ReflectionUtils.newInstance(codecClass, job);
				Compressor compressor = CodecPool.getCompressor(codec);
				compressor.reset();
				CompressionOutputStream compressedOut = codec.createOutputStream(out, compressor);
				out = new FSDataOutputStream(compressedOut,  null);
			}

			Iterator<Record<K, V>> iter = null;
			if (job.getCombinerClass() != null) {
				iter = new CombineRecordIterator<K, V>(job, new RecordIterator(this.records), this.comparator);
			} else iter = this.records.iterator();

			while (iter.hasNext()) {
				Record<K, V> record = iter.next();
				record.marshall(job);
				record.write(out);
			}
			out.close();

			reset();
		}
	}
	
	private class RecordMerger<K extends Object, V extends Object> implements Iterator<Record> {
		
		private RawComparator<K> comparator;
		
		private List<RecordQueue<K, V>> queues;
		
		public RecordMerger(RawComparator<K> comparator, List<RecordQueue<K, V>> queues) {
			this.comparator = comparator;
			this.queues = queues;
		}
		
		@Override
		public Record next() {
			RecordQueue<K, V> next = null;
			for (RecordQueue<K, V> queue : queues) {
				if (queue.hasNext()) {
					if (next == null) {
						next = queue;
					}
					else if (comparator.compare(queue.peek().key, next.peek().key) < 0) {
						next = queue;
					}
				}
			}
			return next != null ? next.remove() : null;
		}

		@Override
		public boolean hasNext() {
			for (RecordQueue<K, V> queue : queues) {
				if (queue.hasNext()) return true;
			}
			return false;
		}

		@Override
		public void remove() {
			/* Not supported */
		}

	}
	
	private static class RecordStream<K extends Object, V extends Object> implements Iterator<Record<K, V>> {
		private boolean done;
		
		private List<Record> cache;
		
		private List<Record.RecordQueue<K, V>> queues;
		
		private Record.RecordQueue<K, V> current;
		
		public RecordStream(List<Record.RecordQueue<K, V>> queues) {
			this.cache = new ArrayList<Record>();
			this.done = false;
			this.queues = queues;
			this.current = queues.size() > 0 ? queues.remove(0) : null;
		}
		
		public void add(Record record) {
			synchronized (cache) {
				this.cache.add(record);
				this.cache.notify();
			}
		}
		
		public void done(boolean value) {
			synchronized (cache) {
				this.done = value;
				this.cache.notify();
			}
		}

		@Override
		public boolean hasNext() {
			synchronized (cache) {
				if (this.current != null && this.current.hasNext()) return true;
				
				while (!done && this.cache.size() == 0) {
					try {
						this.cache.wait();
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				return !done || this.cache.size() > 0;
			}
		}

		@Override
		public Record<K, V> next() {
			/* Drain cache first */
			synchronized (cache) {
				if (cache.size() > 0) {
					Record record = cache.remove(0);
					return record;
				}
				else if (this.current != null && this.current.hasNext()) {
					return this.current.remove();
				}
				else if (this.queues.size() != 0) {
					this.current = this.queues.remove(0);
					return next();
				}
				else {
					this.current = null;
				}
			}
			return hasNext() ? next() : null;
		}

		@Override
		public void remove() {
			/* Not supported. */
		}
		
	}
	
	private static class RecordIterator<K extends Object, V extends Object> implements Record.RecordQueue<K, V> {
		
		private Iterator<Record> recordIterator;
		
		private Record current;
		
		public RecordIterator(List<Record> recordList) {
			this.recordIterator = recordList.iterator();
			this.current = recordIterator.hasNext() ? recordIterator.next() : null;
		}

		@Override
		public boolean hasNext() {
			return current != null;
		}

		@Override
		public Record<K, V> peek() {
			return current;
		}

		@Override
		public Record<K, V> remove() {
			Record record = this.current;
			this.current = recordIterator.hasNext() ? recordIterator.next() : null;
			return record;
		}
	}
	
	private class CombineRecordIterator<K extends Object, V extends Object> implements Iterator<Record<K, V>> {

		private Record.RecordQueue<K, V> records;

		private RawComparator<K> comparator;

		private Reducer combiner;

		public CombineRecordIterator(JobConf job, Record.RecordQueue<K, V> records, RawComparator<K> comparator) {
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
			this.records.remove();
		}

		private Record<K, V> combineNext() throws IOException {
			if (!this.records.hasNext()) return null;

			List<V> values = new ArrayList<V>();
			Record<K, V> next = this.records.remove();
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

	private static class FSRecordIterator<K extends Object, V extends Object> implements Record.RecordQueue<K, V> {
		
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
		public Record<K, V> remove() {
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
	}
}
