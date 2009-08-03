package org.apache.hadoop.bufmanager;

import java.io.DataInputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.UnknownHostException;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.CompressionOutputStream;
import org.apache.hadoop.io.compress.Compressor;
import org.apache.hadoop.io.compress.Decompressor;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapOutputFile;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.util.IndexedSortable;
import org.apache.hadoop.util.IndexedSorter;
import org.apache.hadoop.util.QuickSort;
import org.apache.hadoop.util.ReflectionUtils;

public class JBuffer<K extends Object, V extends Object> extends Thread implements BufferTransfer<K, V> {
	
	protected final JobConf job;
	
	protected final Executor executor;
	
	protected BufferUmbilicalProtocol umbilical;
	
	protected BufferID bufid;
	
    private MapOutputFile mapOutputFile;
	
	private RecordList<K, V> cache;
	
    private final FileSystem localFs;
    
    private List<Path> spillFiles;

    private long curMemorySize;
    
    protected BufferType type;
    
    protected boolean closed;
    
    private Set<BufferRequest> requests;
    
    private Set<RecordStream<K, V>> streams;
    
	public JBuffer(BufferUmbilicalProtocol umbilical, Executor executor, JobConf job, BufferID bufid, BufferType type) throws IOException {
		this.umbilical = umbilical;
		this.executor = executor;
		this.job = job;
		this.bufid = bufid;
		this.type = type;
		this.closed = false;
		
		this.requests = new HashSet<BufferRequest>();
		this.streams = new HashSet<RecordStream<K, V>>();
		
	    /* Setup the class loader */
		/*
		URL jar = new URL("file", null, job.getJar());
		URL [] jars = {jar};
		ClassLoader loader = new URLClassLoader(jars);
		job.setClassLoader(loader);
		*/
		
	    this.localFs = FileSystem.getLocal(job);
		this.mapOutputFile = new MapOutputFile(bufid.taskid().getJobID());
		this.mapOutputFile.setConf(job);
		
	    this.spillFiles = new ArrayList<Path>();
	    
	    reset();
	}
	
	@Override
	public int hashCode() {
		return this.bufid.hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof JBuffer) {
			return this.bufid.equals(((JBuffer)o).bufid);
		}
		return false;
	}
	
	public BufferType type() {
		return this.type;
	}
	
	public BufferID bufid() {
		return this.bufid;
	}
	
	private void reset() {
		this.cache = new RecordList<K, V>(conf());
		this.curMemorySize = 0;
	}
	
	public JobConf conf() {
		return this.job;
	}
	
	protected CompressionCodec codec() {
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
	
	@Override
	public final void add(Record record) throws IOException {
		synchronized (this) {
			if (closed) {
				throw new IOException("Buffer is closed!");
			}
			if (record.marshalled()) record.unmarshall(job);
			else record.marshall(job);
			
			this.cache.add(record);
			this.curMemorySize += record.size();
			
			for (RecordStream stream : streams) {
				stream.add(record);
			}
		}
	}
	
	@Override
	public void transfer(BufferID bufid, String source) throws IOException {
		/* no receivers */
	}
	
	@Override
	public void cancel(BufferID requestID) throws IOException {
		synchronized (this) {
			if (this.bufid.equals(requestID)) {
				for (BufferRequest request : requests) {
					request.cancel();
				}
			}
		}
	}
	
	@Override
	public void done(BufferRequest request) {
		synchronized (this) {
			this.requests.remove(request);
			this.notifyAll();
		}
	}

	@Override
	public void done(BufferReceiver receiver) {
		/* no receivers */
	}
	
	
	@Override
	public void run() {
		if (type() == BufferType.UNSORTED) {
			while (! closed) {
				try {
					BufferRequest request = umbilical.getRequest(bufid());
					if (request == null) {
						sleep(100);
					}
					else {
						request.open(this.bufid, this);
						synchronized (this) {
							if (! closed) {
								this.requests.add(request);
								this.executor.execute(request);
							}
							else {
								request.cancel();
							}
						}
					}
				} catch (IOException e) {
					e.printStackTrace();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	/**
	 * Get an iterator to the records in this buffer.
	 * @param sorted Sorted order by key?
	 * @return Record iterator
	 * @throws IOException 
	 */
	public Iterator<Record<K, V>> iterator() throws IOException {
		synchronized (this) {
			while (this.type == BufferType.SORTED && !closed) {
				try { this.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			
			if (this.type == BufferType.SORTED) {
				this.cache.sort();
			}
			
			List<Record.RecordIterator<K, V>> queues = new ArrayList<Record.RecordIterator<K, V>>();
			
			queues.add(this.cache.iterator());
			for (Path file : this.spillFiles) {
				FSDataInputStream in = localFs.open(file);
				queues.add(new FSRecordIterator(this.job, in));
			}
			
			if (this.type == BufferType.SORTED) {
				return new RecordMerger(this.job.getOutputKeyComparator(), queues);
			}
			else {
				RecordStream stream = new RecordStream(queues);
				this.streams.add(stream);
				return stream;
			}
		}
	}
	
	
	
	@Override
	public void close() throws IOException {
		if (closed) return; // already closed
		
		synchronized (this) {
			this.closed = true;
			for (RecordStream stream : streams) {
				stream.done(true);
			}
			
			/* Don't close until all requests have been satisfied */
			while (this.requests.size() > 0) {
				try {
					this.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			this.closed = true;
		}
	}
	
	public void commit() throws IOException {
		synchronized (this) {
			if (!closed) throw new IOException ("Can't sort and flush until closed!");

			Path filename = mapOutputFile.getOutputFileForWrite(this.bufid.taskid(), Integer.MAX_VALUE);
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

			Iterator<Record<K, V>> iterator = iterator();
			while (iterator.hasNext()) {
				iterator.next().write(out);
			}
			Record.NULL_RECORD.write(out);
			out.close();

			this.spillFiles.clear();
			this.spillFiles.add(filename);
			
			this.umbilical.register(bufid, filename.toString());
		}
	}
	
	public void flush() throws IOException {
		synchronized (this) {
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

			if (this.type == BufferType.SORTED) {
				this.cache.sort();
			}
			
			this.cache.write(out);
			out.close();

			reset();
		}
	}
	
	private class RecordMerger<K extends Object, V extends Object> implements Iterator<Record> {
		
		private RawComparator<K> comparator;
		
		private List<Record.RecordIterator<K, V>> queues;
		
		public RecordMerger(RawComparator<K> comparator, List<Record.RecordIterator<K, V>> queues) {
			this.comparator = comparator;
			this.queues = queues;
		}
		
		@Override
		public Record next() {
			Record.RecordIterator<K, V> iter = null;
			for (Record.RecordIterator<K, V> queue : queues) {
				if (queue.hasNext()) {
					if (iter == null) {
						iter = queue;
					}
					else if (comparator.compare(queue.peek().key, iter.peek().key) < 0) {
						iter = queue;
					}
				}
			}
			return iter != null ? iter.next() : null;
		}

		@Override
		public boolean hasNext() {
			for (Record.RecordIterator<K, V> queue : queues) {
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
		
		private List<Record.RecordIterator<K, V>> queues;
		
		private Record.RecordIterator<K, V> current;
		
		public RecordStream(List<Record.RecordIterator<K, V>> queues) {
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
					Record record = this.current.next();
					return record;
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

}
