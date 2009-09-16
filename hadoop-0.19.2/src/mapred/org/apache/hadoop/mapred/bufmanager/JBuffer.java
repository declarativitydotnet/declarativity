package org.apache.hadoop.mapred.bufmanager;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.SerializationFactory;
import org.apache.hadoop.io.serializer.Serializer;
import org.apache.hadoop.mapred.IFile;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapOutputFile;
import org.apache.hadoop.mapred.Merger;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Partitioner;
import org.apache.hadoop.mapred.RawKeyValueIterator;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.Merger.Segment;
import org.apache.hadoop.util.IndexedSortable;
import org.apache.hadoop.util.IndexedSorter;
import org.apache.hadoop.util.Progress;
import org.apache.hadoop.util.QuickSort;
import org.apache.hadoop.util.ReflectionUtils;

public class JBuffer<K extends Object, V extends Object>  implements JBufferCollector<K, V>, MapOutputCollector<K, V>, IndexedSortable {

	protected static class CombineOutputCollector<K extends Object, V extends Object> 
	implements OutputCollector<K, V> {
		private IFile.Writer<K, V> writer = null;
		
		public CombineOutputCollector() {
		}
		public synchronized void setWriter(IFile.Writer<K, V> writer) {
			this.writer = writer;
		}
		
		public synchronized void reset() {
			this.writer = null;
		}
		
		public synchronized void collect(K key, V value)
		throws IOException {
			if (writer != null) writer.append(key, value);
		}
	}
	
	
	private class SpillThread extends Thread {
		private boolean spill = false;
		private boolean open = true;

		public void doSpill() {
			synchronized (spillLock) {
				if (open) {
					this.spill = true;
					spillLock.notifyAll();
				}
			}
		}
		
		public boolean isSpilling() {
			return this.spill;
		}

		public void close() {
			synchronized (spillLock) {
				this.open = false;
				spillLock.notifyAll();
			}
		}

		@Override
		public void run() {
			try {
				while (open) {
					synchronized (spillLock) {
						while (open && ! spill) {
							try {
								spillLock.wait();
							} catch (InterruptedException e) {
								return;
							}
						}
					}

					if (open) {
						long starttime = java.lang.System.currentTimeMillis();
						try {
								long sortstart = java.lang.System.currentTimeMillis();
								sortAndSpill();
								LOG.debug("SpillThread: sort/spill time " + 
										((System.currentTimeMillis() - sortstart)/1000f) + " secs.");
						} catch (Throwable e) {
							e.printStackTrace();
							sortSpillException = e;
						} finally {
							synchronized(spillLock) {
								if (bufend < bufindex && bufindex < bufstart) {
									bufvoid = kvbuffer.length;
								}
								kvstart = kvend;
								bufstart = bufend;
								spill = false;
								spillLock.notifyAll();
							}
							
							try {
								synchronized (pipelineThread) {
									pipelineThread.notifyAll();
								}
								
								synchronized (mergeLock) {
									mergeLock.notifyAll();
								}
							} finally {
								LOG.debug("SpillThread: total spill time " + 
										((System.currentTimeMillis() - starttime)/1000f) + " secs.");
							}
						}
					}
				}
			} finally {
				synchronized (spillLock) {
					open  = false;
					spill = false;
					spillLock.notifyAll();
				}
			}
		}
	}
	
	private class MergeThread extends Thread {
		private boolean open = true;
		private boolean busy = false;

		public void close() {
			synchronized (mergeLock) {
				open = false;
				while (busy) {
					try { mergeLock.wait();
					} catch (InterruptedException e) { }
				}
				this.interrupt();
			}
		}
		
		public boolean isBusy() {
			return busy;
		}


		@Override
		public void run() {
			try {
				while (!isInterrupted()) {
					synchronized (mergeLock) {
						busy = false;
						while (open && numSpills - numFlush < 100) {
							try {
								mergeLock.wait();
							} catch (InterruptedException e) {
								return;
							}
						}
						if (!open) return;
						busy = true;
					}

					if (!taskid.isMap() && numSpills - numFlush > 100) {
						try {
							long mergestart = java.lang.System.currentTimeMillis();
							mergeParts(true);
							LOG.debug("SpillThread: merge time " +  ((System.currentTimeMillis() - mergestart)/1000f) + " secs.");
						} catch (IOException e) {
							e.printStackTrace();
							sortSpillException = e;
							return; // dammit
						}
					}
				}
			} finally {
				synchronized (this) {
					open  = false;
					this.notifyAll();
				}
			}
		}
	}
	
	private class PipelineThread extends Thread {
		private boolean open = false;
		private boolean busy = false;
		
		public void close() {
			synchronized (this) {
				open = false;
				while (busy) {
					try { this.wait();
					} catch (InterruptedException e) { }
				}
				this.interrupt();
			}
		}
		
		public void open() {
			synchronized (this) {
				if (open) return;
				else {
					start();
					while (!open) {
						try { this.wait();
						} catch (InterruptedException e) { }
					}
				}
			}
		}
		
		public void run() {
			int flushpoint = numFlush;
			synchronized (this) {
				open = true;
				this.notifyAll();
			}
			
			try {
				while (!isInterrupted()) {
					synchronized (this) {
						busy = false;
						while (open && flushpoint == numSpills) {
							try { this.wait();
							} catch (InterruptedException e) {
								return;
							}
						}
						if (!open) return;
						busy = true;
					}

					try {
						long pipelinestart = java.lang.System.currentTimeMillis();
						flushpoint = flushRequests();
						LOG.debug("PipelinMergeThread: pipeline time " +  
								((System.currentTimeMillis() - pipelinestart)/1000f) + " secs.");
					} catch (IOException e) {
						e.printStackTrace();
						sortSpillException = e;
					}
				}
			} finally {
				synchronized (this) {
					this.notifyAll();
				}
			}
		}
			
		private int flushRequests() throws IOException {
			int spillend = numSpills;

			BufferRequest request = null;
			while ((request = umbilical.getRequest(taskid)) != null) {
				if (request.open(job, false)) {
					requests.add(request);
					requestMap.put(request.partition(), request); // TODO speculation
				}
			}
			
			if (numSpills == numFlush) return spillend;

			/* Iterate over spill files in order. */
			for (int spillId = numFlush; spillId < spillend; spillId++) {
				FSDataInputStream indexIn = null;
				FSDataInputStream dataIn  = null;
				for (BufferRequest r : requests) {
					if (r.flushPoint() < spillId) {
						if (dataIn == null) {
							Path outputFile = mapOutputFile.getSpillFile(taskid, spillId);
							Path indexFile  = mapOutputFile.getSpillIndexFile(taskid, spillId);
							indexIn = localFs.open(indexFile);
							dataIn = localFs.open(outputFile);
						}
						
						float requestProgress = (spillId / (float) numSpills) * progress.get();
						r.flush(indexIn, dataIn, spillId, requestProgress);
					}
				}
				if (dataIn != null) {
					indexIn.close(); indexIn = null;
					dataIn.close();  dataIn = null;
				}
			}
			
			
			/* Rate limit the data pipeline. */
			if (requests.size() > partitions / 2 && numSpills - spillend > 5) {
				float avgDataRate = 0f;
				BufferRequest min = null;
				for (BufferRequest r : requests) {
					if (r.datarate() < Float.MIN_VALUE) continue;
					
					avgDataRate += r.datarate();
					if (min == null || r.datarate() < min.datarate()) {
						min = r;
					}
				}
				avgDataRate /= (float) requests.size();
				
				if (min != null && (avgDataRate / min.datarate()) > 10.0) {
					LOG.warn("Pipeline running slow! Min data rate = " + 
							            min.datarate() + ". Average data rate = " + avgDataRate);
					min.close();
					requests.remove(min);
					requestMap.remove(min.partition());
				}
			}

			if (requests.size() == partitions) {
				numFlush = spillend;
			}

			return spillend;
		}
	}
	
	private static final Log LOG = LogFactory.getLog(JBuffer.class.getName());

	
    private Progress progress;

	/**
	 * The size of each record in the index file for the map-outputs.
	 */
	public static final int MAP_OUTPUT_INDEX_RECORD_LENGTH = 24;

	private final static int APPROX_HEADER_LENGTH = 150;

	private final BufferUmbilicalProtocol umbilical;

	private final int partitions;
	private final Partitioner<K, V> partitioner;
	private final JobConf job;
	private final TaskAttemptID taskid;
	private final Reporter reporter;
	private final Class<K> keyClass;
	private final Class<V> valClass;
	private final RawComparator<K> comparator;
	private final SerializationFactory serializationFactory;
	private final Serializer<K> keySerializer;
	private final Serializer<V> valSerializer;
	private final Class<? extends Reducer> combinerClass;
	private final CombineOutputCollector<K, V> combineCollector;

	// Compression for map-outputs
	private CompressionCodec codec = null;

	// k/v accounting
	private volatile int kvstart = 0;  // marks beginning of spill
	private volatile int kvend = 0;    // marks beginning of collectable
	private int kvindex = 0;           // marks end of collected
	private final int[] kvoffsets;     // indices into kvindices
	private final int[] kvindices;     // partition, k/v offsets into kvbuffer
	private volatile int bufstart = 0; // marks beginning of spill
	private volatile int bufend = 0;   // marks beginning of collectable
	private volatile int bufvoid = 0;  // marks the point where we should stop
	// reading at the end of the buffer
	private int bufindex = 0;          // marks end of collected
	private int bufmark = 0;           // marks end of record
	private byte[] kvbuffer;           // main output buffer
	private static final int PARTITION = 0; // partition offset in acct
	private static final int KEYSTART = 1;  // key offset in acct
	private static final int VALSTART = 2;  // val offset in acct
	private static final int ACCTSIZE = 3;  // total #fields in acct
	private static final int RECSIZE =
		(ACCTSIZE + 1) * 4;  // acct bytes per record

	// spill accounting
	private volatile int numFlush = 0;
	private volatile int numSpills = 0;
	private volatile Throwable sortSpillException = null;
	private final int softRecordLimit;
	private final int softBufferLimit;
	private final int minSpillsForCombine;
	private final IndexedSorter sorter;
	private final Object spillLock = new Object();
	private final Object mergeLock = new Object();
	private final BlockingBuffer bb = new BlockingBuffer();
	
	private long reserve = 0;

	private final FileSystem localFs;

	private MapOutputFile mapOutputFile = null;
	
	private boolean pipeline;
	private PipelineThread pipelineThread;
	
	private SpillThread spillThread;
	private MergeThread mergeThread;
	
	private TreeSet<BufferRequest> requests = new TreeSet<BufferRequest>();
	private Map<Integer, BufferRequest> requestMap = new HashMap<Integer, BufferRequest>();
	
	@SuppressWarnings("unchecked")
	public JBuffer(BufferUmbilicalProtocol umbilical, TaskAttemptID taskid, JobConf job, Reporter reporter) throws IOException {
		this.umbilical = umbilical;
		this.taskid = taskid;
		this.job = job;
		this.reporter = reporter;
		this.mapOutputFile = new MapOutputFile(taskid.getJobID());
		this.mapOutputFile.setConf(job);
		
		this.progress = new Progress();
		this.progress.set(0f);
		
		this.pipelineThread = new PipelineThread();
		this.pipelineThread.setDaemon(true);
		
		this.spillThread = new SpillThread();
		this.spillThread.setDaemon(true);
		this.spillThread.start();
		
		this.mergeThread = new MergeThread();
		this.mergeThread.setDaemon(true);
		if (!taskid.isMap()) this.mergeThread.start();
		
		localFs = FileSystem.getLocal(job);
		partitions = taskid.isMap() ? job.getNumReduceTasks() : 1;
		partitioner = (Partitioner)
		ReflectionUtils.newInstance(job.getPartitionerClass(), job);
		// sanity checks
		final float spillper = job.getFloat("io.sort.spill.percent",(float)0.8);
		final float recper = job.getFloat("io.sort.record.percent",(float)0.05);
		final int sortmb = job.getInt("io.sort.mb", 100);
		if (spillper > (float)1.0 || spillper < (float)0.0) {
			throw new IOException("Invalid \"io.sort.spill.percent\": " + spillper);
		}
		if (recper > (float)1.0 || recper < (float)0.01) {
			throw new IOException("Invalid \"io.sort.record.percent\": " + recper);
		}
		if ((sortmb & 0x7FF) != sortmb) {
			throw new IOException("Invalid \"io.sort.mb\": " + sortmb);
		}
		sorter = (IndexedSorter)
		ReflectionUtils.newInstance(
				job.getClass("map.sort.class", QuickSort.class), job);
		// buffers and accounting
		int maxMemUsage = sortmb << 20;
		
		if (!taskid.isMap()) {
	        float maxInMemCopyUse = job.getFloat("mapred.job.shuffle.input.buffer.percent", 0.70f);
		    maxMemUsage = (int)Math.min(Runtime.getRuntime().maxMemory() * maxInMemCopyUse, Integer.MAX_VALUE);
		}
		
		int recordCapacity = (int)(maxMemUsage * recper);
		recordCapacity -= recordCapacity % RECSIZE;
		kvbuffer = new byte[maxMemUsage - recordCapacity];
		bufvoid = kvbuffer.length;
		recordCapacity /= RECSIZE;
		kvoffsets = new int[recordCapacity];
		kvindices = new int[recordCapacity * ACCTSIZE];
		softBufferLimit = (int)(kvbuffer.length * spillper);
		softRecordLimit = (int)(kvoffsets.length * spillper);
		// k/v serialization
		comparator = job.getOutputKeyComparator();
		keyClass = (Class<K>)job.getMapOutputKeyClass();
		valClass = (Class<V>)job.getMapOutputValueClass();
		serializationFactory = new SerializationFactory(job);
		keySerializer = serializationFactory.getSerializer(keyClass);
		keySerializer.open(bb);
		valSerializer = serializationFactory.getSerializer(valClass);
		valSerializer.open(bb);

		// compression
		if (job.getCompressMapOutput()) {
			Class<? extends CompressionCodec> codecClass =
				job.getMapOutputCompressorClass(DefaultCodec.class);
			codec = (CompressionCodec)
			ReflectionUtils.newInstance(codecClass, job);
		}
		// combiner
		combinerClass = job.getCombinerClass();
		combineCollector = (null != combinerClass)
		? new CombineOutputCollector()
		: null;
		minSpillsForCombine = job.getInt("min.num.spills.for.combine", 3);
	}
	
	public byte[] buffer() {
		return this.kvbuffer;
	}
	
	public TreeSet<BufferRequest> requests() {
		return this.requests;
	}
	
	public synchronized void pipeline(boolean value) {
		if (pipeline == false && value == true) {
			this.pipelineThread.open();
		}
		else if (pipeline == true && value == false) {
			this.pipelineThread.close();
		}
		this.pipeline = value;
	}
	
	public Progress getProgress() {
		return this.progress;
	}
	
	public void setProgress(Progress progress) {
		this.progress = progress;
	}
	
	/**
	 * @return The number of free bytes.
	 */
	public int getBytes() {
		return ((kvend > kvstart) ? kvend : kvoffsets.length + kvend) - kvstart;
	}
	
	public synchronized boolean reserve(long bytes) {
		if (kvbuffer == null) return false;
		else if (bytes < getBytes() - this.reserve) {
			this.reserve += bytes;
			return true;
		}
		return false;
	}
	
	public void unreserve(long bytes) {
		this.reserve -= bytes;
		
		if (this.reserve < 0) {
			LOG.error("Reserve bytes error: " + this.reserve);
			this.reserve = 0;
		}
	}

	/**
	 * Optimized collection routine used by the reducer.
	 * @param key
	 * @param value
	 * @throws IOException
	 */
	public synchronized void collect(DataInputBuffer key, DataInputBuffer value) throws IOException {
		if (sortSpillException != null) {
			throw (IOException)new IOException("Spill failed"
			).initCause(sortSpillException);
		}
		else if (this.partitions > 1) {
			throw new IOException("Method not for use with more than one partition");
		}
		
		try {
			int keystart = bufindex;
			bb.write(key.getData(), key.getPosition(), key.getLength() - key.getPosition());
			if (bufindex < keystart) {
				// wrapped the key; reset required
				bb.reset();
				keystart = 0;
			}
			
			// copy value bytes into buffer
			int valstart = bufindex;
			bb.write(value.getData(), value.getPosition(), value.getLength() - value.getPosition());

			if (keystart == bufindex) {
				// if emitted records make no writes, it's possible to wrap
				// accounting space without notice
				bb.write(new byte[0], 0, 0);
			}

			// update accounting info
			int ind = kvindex * ACCTSIZE;
			kvoffsets[kvindex] = ind;
			kvindices[ind + PARTITION] = 0;
			kvindices[ind + KEYSTART] = keystart;
			kvindices[ind + VALSTART] = valstart;
			kvindex = (kvindex + 1) % kvoffsets.length;
		} catch (MapBufferTooSmallException e) {
			// LOG.info("Record too large for in-memory buffer: " + e.getMessage());
			spillSingleRecord(key, value);
			return;
		}
	}
	
	@SuppressWarnings("unchecked")
	public synchronized void collect(K key, V value)
	throws IOException {
		reporter.progress();
		if (key.getClass() != keyClass) {
			throw new IOException("Type mismatch in key from map: expected "
					+ keyClass.getName() + ", recieved "
					+ key.getClass().getName());
		}
		if (value.getClass() != valClass) {
			throw new IOException("Type mismatch in value from map: expected "
					+ valClass.getName() + ", recieved "
					+ value.getClass().getName());
		}
		if (sortSpillException != null) {
			throw (IOException)new IOException("Spill failed"
			).initCause(sortSpillException);
		}
		try {
			// serialize key bytes into buffer
			int keystart = bufindex;
			keySerializer.serialize(key);
			if (bufindex < keystart) {
				// wrapped the key; reset required
				bb.reset();
				keystart = 0;
			}
			// serialize value bytes into buffer
			int valstart = bufindex;
			valSerializer.serialize(value);
			int valend = bb.markRecord();

			if (keystart == bufindex) {
				// if emitted records make no writes, it's possible to wrap
				// accounting space without notice
				bb.write(new byte[0], 0, 0);
			}

			int partition = partitioner.getPartition(key, value, partitions);
			if (partition < 0 || partition >= partitions) {
				throw new IOException("Illegal partition for " + key + " (" +
						partition + ")");
			}

			// update accounting info
			int ind = kvindex * ACCTSIZE;
			kvoffsets[kvindex] = ind;
			kvindices[ind + PARTITION] = partition;
			kvindices[ind + KEYSTART] = keystart;
			kvindices[ind + VALSTART] = valstart;
			kvindex = (kvindex + 1) % kvoffsets.length;
		} catch (MapBufferTooSmallException e) {
			// LOG.info("Record too large for in-memory buffer: " + e.getMessage());
			spillSingleRecord(key, value);
			return;
		}

	}

	/**
	 * Compare logical range, st i, j MOD offset capacity.
	 * Compare by partition, then by key.
	 * @see IndexedSortable#compare
	 */
	public int compare(int i, int j) {
		final int ii = kvoffsets[i % kvoffsets.length];
		final int ij = kvoffsets[j % kvoffsets.length];
		// sort by partition
		if (kvindices[ii + PARTITION] != kvindices[ij + PARTITION]) {
			return kvindices[ii + PARTITION] - kvindices[ij + PARTITION];
		}
		// sort by key
		return comparator.compare(kvbuffer,
				kvindices[ii + KEYSTART],
				kvindices[ii + VALSTART] - kvindices[ii + KEYSTART],
				kvbuffer,
				kvindices[ij + KEYSTART],
				kvindices[ij + VALSTART] - kvindices[ij + KEYSTART]);
	}

	/**
	 * Swap logical indices st i, j MOD offset capacity.
	 * @see IndexedSortable#swap
	 */
	public void swap(int i, int j) {
		i %= kvoffsets.length;
		j %= kvoffsets.length;
		int tmp = kvoffsets[i];
		kvoffsets[i] = kvoffsets[j];
		kvoffsets[j] = tmp;
	}

	/**
	 * Inner class managing the spill of serialized records to disk.
	 */
	protected class BlockingBuffer extends DataOutputStream {

		public BlockingBuffer() {
			this(new Buffer());
		}

		private BlockingBuffer(OutputStream out) {
			super(out);
		}

		/**
		 * Mark end of record. Note that this is required if the buffer is to
		 * cut the spill in the proper place.
		 */
		public int markRecord() {
			bufmark = bufindex;
			return bufindex;
		}

		/**
		 * Set position from last mark to end of writable buffer, then rewrite
		 * the data between last mark and kvindex.
		 * This handles a special case where the key wraps around the buffer.
		 * If the key is to be passed to a RawComparator, then it must be
		 * contiguous in the buffer. This recopies the data in the buffer back
		 * into itself, but starting at the beginning of the buffer. Note that
		 * reset() should <b>only</b> be called immediately after detecting
		 * this condition. To call it at any other time is undefined and would
		 * likely result in data loss or corruption.
		 * @see #markRecord()
		 */
		protected synchronized void reset() throws IOException {
			// spillLock unnecessary; If spill wraps, then
			// bufindex < bufstart < bufend so contention is impossible
			// a stale value for bufstart does not affect correctness, since
			// we can only get false negatives that force the more
			// conservative path
			int headbytelen = bufvoid - bufmark;
			bufvoid = bufmark;
			if (bufindex + headbytelen < bufstart) {
				System.arraycopy(kvbuffer, 0, kvbuffer, headbytelen, bufindex);
				System.arraycopy(kvbuffer, bufvoid, kvbuffer, 0, headbytelen);
				bufindex += headbytelen;
			} else {
				byte[] keytmp = new byte[bufindex];
				System.arraycopy(kvbuffer, 0, keytmp, 0, bufindex);
				bufindex = 0;
				out.write(kvbuffer, bufmark, headbytelen);
				out.write(keytmp);
			}
		}
	}

	public class Buffer extends OutputStream {
		private final byte[] scratch = new byte[1];

		@Override
		public synchronized void write(int v)
		throws IOException {
			scratch[0] = (byte)v;
			write(scratch, 0, 1);
		}

		/**
		 * Attempt to write a sequence of bytes to the collection buffer.
		 * This method will block if the spill thread is running and it
		 * cannot write.
		 * @throws MapBufferTooSmallException if record is too large to
		 *    deserialize into the collection buffer.
		 */
		@Override
		public synchronized void write(byte b[], int off, int len)
		throws IOException {
			boolean kvfull = false;
			boolean buffull = false;
			boolean wrap = false;
			synchronized(spillLock) {
				do {
					if (sortSpillException != null) {
						throw (IOException)new IOException("Spill failed"
						).initCause(sortSpillException);
					}

					// sufficient accounting space?
					final int kvnext = (kvindex + 1) % kvoffsets.length;
					kvfull = kvnext == kvstart;
					// sufficient buffer space?
					if (bufstart <= bufend && bufend <= bufindex) {
						buffull = bufindex + len > bufvoid;
						wrap = (bufvoid - bufindex) + bufstart > len;
					} else {
						// bufindex <= bufstart <= bufend
						// bufend <= bufindex <= bufstart
						wrap = false;
						buffull = bufindex + len > bufstart;
					}

					if (kvstart == kvend) {
						// spill thread not running
						if (kvend != kvindex) {
							// we have records we can spill
							final boolean kvsoftlimit = (kvnext > kvend)
							? kvnext - kvend > softRecordLimit
									: kvend - kvnext <= kvoffsets.length - softRecordLimit;
							final boolean bufsoftlimit = (bufindex > bufend)
							? bufindex - bufend > softBufferLimit
									: bufend - bufindex < bufvoid - softBufferLimit;
							if (kvsoftlimit || bufsoftlimit || (buffull && !wrap)) {
								/*
                  LOG.info("Spilling map output: buffer full = " + bufsoftlimit+
                           " and record full = " + kvsoftlimit);
                  LOG.info("bufstart = " + bufstart + "; bufend = " + bufmark +
                           "; bufvoid = " + bufvoid);
                  LOG.info("kvstart = " + kvstart + "; kvend = " + kvindex +
                           "; length = " + kvoffsets.length);
								 */
								kvend = kvindex;
								bufend = bufmark;
								// TODO No need to recreate this thread every time
								spillThread.doSpill();
							}
						} else if (buffull && !wrap) {
							// We have no buffered records, and this record is too large
							// to write into kvbuffer. We must spill it directly from
							// collect
							final int size = ((bufend <= bufindex)
									? bufindex - bufend
											: (bufvoid - bufend) + bufindex) + len;
							bufstart = bufend = bufindex = bufmark = 0;
							kvstart = kvend = kvindex = 0;
							bufvoid = kvbuffer.length;
							throw new MapBufferTooSmallException(size + " bytes");
						}
					}

					if (kvfull || (buffull && !wrap)) {
						while (kvstart != kvend) {
							reporter.progress();
							try {
								spillLock.wait();
							} catch (InterruptedException e) {
								throw (IOException)new IOException(
										"Buffer interrupted while waiting for the writer"
								).initCause(e);
							}
						}
					}
				} while (kvfull || (buffull && !wrap));
			}
			// here, we know that we have sufficient space to write
			if (buffull) {
				final int gaplen = bufvoid - bufindex;
				System.arraycopy(b, off, kvbuffer, bufindex, gaplen);
				len -= gaplen;
				off += gaplen;
				bufindex = 0;
			}
			System.arraycopy(b, off, kvbuffer, bufindex, len);
			bufindex += len;
		}
	}
	
	public boolean canSnapshot() throws IOException {
		if (pipeline) throw new IOException("Snapshot not allowed with pipelineing!");
		BufferRequest request = null;
		while ((request = umbilical.getRequest(taskid)) != null) {
			if (request.open(job, true)) {
				requests.add(request);
				requestMap.put(request.partition(), request); // TODO speculation
			}
		}

		return (requests.size() > 0);
	}
	
	public synchronized boolean snapshot(boolean reset) throws IOException {

		try {
			synchronized (mergeLock) {
				if (!canSnapshot()) {
					return true; // pretend i did it.
				}
				
				if (numFlush == numSpills) {
					synchronized (spillLock) {
						spillThread.doSpill();
						while (kvstart != kvend) {
							reporter.progress();
							try {
								spillLock.wait();
							} catch (InterruptedException e) {
								throw (IOException)new IOException(
										"Buffer interrupted while waiting for the writer"
								).initCause(e);
							}
						}
					}
				}
				
				int spillId = mergeParts(true);
				if (spillId < 0) return true;

				Path snapFile = mapOutputFile.getSpillFile(this.taskid, spillId);
				Path indexFile = mapOutputFile.getSpillIndexFile(this.taskid, spillId);

				FSDataInputStream indexIn = localFs.open(indexFile);
				FSDataInputStream dataIn  = localFs.open(snapFile);
				for (BufferRequest r : requests) {
					System.err.println("JBuffer: do snapshot request " + r);
					r.flush(indexIn, dataIn, -1, progress.get());
				}
				if (reset) {
					reset(true);
					localFs.delete(snapFile, true);
					localFs.delete(indexFile, true);
				}
			}
			return true;
		} catch (Throwable t) {
			return false; // Turn off snapshots.
		}
	}
	
	public synchronized ValuesIterator<K, V> iterator() throws IOException {
		/*
		if (this.numSpills == 0) {
			int endPosition = (kvend > kvstart)
			? kvend : kvoffsets.length + kvend;
			sorter.sort(JBuffer.this, kvstart, endPosition, reporter);

			RawKeyValueIterator kvIter =
				new MRResultIterator(kvstart, endPosition);
			return new ValuesIterator<K, V>(kvIter, comparator, keyClass, valClass, job, reporter);
		}
		*/
		Path finalOutputFile = mapOutputFile.getOutputFile(this.taskid);
		RawKeyValueIterator kvIter = new FSMRResultIterator(this.localFs, finalOutputFile);
		return new ValuesIterator<K, V>(kvIter, comparator, keyClass, valClass, job, reporter);
	}

	public synchronized void flush() throws IOException {
		if (numSpills == 0 && kvend == kvindex) {
			LOG.warn("JBuffer: no flush needed for buffer " + taskid);
			return;
		}
		
		pipelineThread.close();
		mergeThread.close();
		
		synchronized (spillLock) {
			this.spillThread.close();
			while (this.spillThread.isSpilling()) {
				try {
					spillLock.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
					throw (IOException)new IOException(
							"Buffer " + taskid + " interrupted while waiting for the writer"
					).initCause(e);
				}
			}
		}
			
		if (sortSpillException != null) {
			throw (IOException)new IOException("Spill failed"
			).initCause(sortSpillException);
		}
		if (kvend != kvindex) {
			kvend = kvindex;
			bufend = bufmark;
			sortAndSpill();
		}
		
		mergeParts(false);
		reset(false);
	}
	
	public synchronized void reset(boolean restart) throws IOException {
		synchronized (spillLock) {
			spillThread.close();
			while (spillThread.isSpilling()) {
				try {
					spillLock.wait();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}

			synchronized (mergeLock) {
				mergeThread.close();
				while (mergeThread.isBusy()) {
					try { mergeLock.wait();
					} catch (InterruptedException e) { }
				}
				
				/* reset buffer variables. */
				numSpills = numFlush = 0;
				bufindex = 0;
				bufvoid  = kvbuffer.length;
				kvstart = kvend = kvindex = 0;  
				bufstart = bufend = bufvoid = bufindex = bufmark = 0; 
				
				this.progress = new Progress();
				
				if (restart) {
					/* restart threads. */
					this.spillThread = new SpillThread();
					this.spillThread.setDaemon(true);
					this.spillThread.start();

					this.mergeThread = new MergeThread();
					this.mergeThread.setDaemon(true);
					if (!taskid.isMap()) this.mergeThread.start();
				}
			}
		}
	}

	public void close() throws IOException {  
		flush();
	}
	
	public void free() {
		kvbuffer = null;
		for (BufferRequest request : requests) {
			try {
				request.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		requests.clear();
	}
	
	public void spill(Path data, long size, Path index, boolean copy) throws IOException {
		Path dataFile = null;
		Path indexFile = null;
		synchronized (mergeLock) {
			dataFile  = mapOutputFile.getSpillFileForWrite(this.taskid, this.numSpills, size);
			indexFile = mapOutputFile.getSpillIndexFileForWrite(this.taskid, this.numSpills, partitions * MAP_OUTPUT_INDEX_RECORD_LENGTH);
			numSpills++;

			if (copy) {
				localFs.copyFromLocalFile(data, dataFile);
				localFs.copyFromLocalFile(index, indexFile);
			}
			else {
				if (!localFs.rename(data, dataFile)) {
					throw new IOException("JBuffer::spill -- unable to rename " + data + " to " + dataFile);
				}
				if (!localFs.rename(index, indexFile)) {
					throw new IOException("JBuffer::spill -- unable to rename " + index + " to " + indexFile);
				}
			}
			
			mergeLock.notifyAll();
		}
	}
	

	private void sortAndSpill() throws IOException {
		//approximate the length of the output file to be the length of the
		//buffer + header lengths for the partitions
		synchronized (mergeLock) {
			long size = (bufend >= bufstart
					? bufend - bufstart
							: (bufvoid - bufend) + bufstart) +
							partitions * APPROX_HEADER_LENGTH;
			FSDataOutputStream out = null;
			FSDataOutputStream indexOut = null;
			try {
				// create spill file
				Path filename = mapOutputFile.getSpillFileForWrite(this.taskid, this.numSpills, size);
				if (localFs.exists(filename)) {
					throw new IOException("JBuffer::sortAndSpill -- spill file exists! " + filename);
				}
				
				out = localFs.create(filename, false);
				if (out == null ) throw new IOException("Unable to create spill file " + filename);
				// create spill index
				Path indexFilename = mapOutputFile.getSpillIndexFileForWrite(
						this.taskid, this.numSpills,
						partitions * MAP_OUTPUT_INDEX_RECORD_LENGTH);
				indexOut = localFs.create(indexFilename, false);

				final int endPosition = (kvend > kvstart)
				? kvend
						: kvoffsets.length + kvend;
				sorter.sort(JBuffer.this, kvstart, endPosition, reporter);
				int spindex = kvstart;
				InMemValBytes value = new InMemValBytes();
				for (int i = 0; i < partitions; ++i) {
					IFile.Writer<K, V> writer = null;
					BufferRequest request = null;
					try {
						long segmentStart = out.getPos();
						writer = new IFile.Writer<K, V>(job, out, keyClass, valClass, codec);

						if (null == combinerClass) {
							// spill directly
							DataInputBuffer key = new DataInputBuffer();
							while (spindex < endPosition
									&& kvindices[kvoffsets[spindex
									                       % kvoffsets.length]
									                       + PARTITION] == i) {
								final int kvoff = kvoffsets[spindex % kvoffsets.length];
								getVBytesForOffset(kvoff, value);
								key.reset(kvbuffer, kvindices[kvoff + KEYSTART], 
										(kvindices[kvoff + VALSTART] - kvindices[kvoff + KEYSTART]));

								writer.append(key, value);
								++spindex;
							}
						} else {
							int spstart = spindex;
							while (spindex < endPosition
									&& kvindices[kvoffsets[spindex % kvoffsets.length] + PARTITION] == i) {
								++spindex;
							}
							// Note: we would like to avoid the combiner if
							// we've fewer
							// than some threshold of records for a partition
							if (spstart != spindex) {
								combineCollector.setWriter(writer);

								RawKeyValueIterator kvIter = new MRResultIterator(spstart, spindex);
								combineAndSpill(kvIter);

								combineCollector.reset();
							}
						}

						// close the writer
						writer.close();

						// write the index as <offset, raw-length,
						// compressed-length>
						writeIndexRecord(indexOut, out, segmentStart, writer);
						writer = null;
					} finally {
						// if (null != request) request.flushBuffer();
						if (null != writer) {
							writer.close();
							writer = null;
						}
					}
				}
				// LOG.info("Finished spill " + numSpills);
				++numSpills;
			} finally {
				if (out != null) out.close();
				if (indexOut != null) indexOut.close();
			}
		}
	}
	
	private void spillSingleRecord(final DataInputBuffer key, final DataInputBuffer value)  throws IOException {
		// TODO this right
		Class keyClass = job.getMapOutputKeyClass();
		Class valClass = job.getMapOutputValueClass();
	    SerializationFactory serializationFactory = new SerializationFactory(job);
	    Deserializer<K> keyDeserializer = serializationFactory.getDeserializer(keyClass);
	    Deserializer<V> valDeserializer = serializationFactory.getDeserializer(valClass);
		keyDeserializer.open(key);
		valDeserializer.open(value);
		K k = keyDeserializer.deserialize(null);
		V v = valDeserializer.deserialize(null);
		spillSingleRecord(k, v);
	}

	/**
	 * Handles the degenerate case where serialization fails to fit in
	 * the in-memory buffer, so we must spill the record from collect
	 * directly to a spill file. Consider this "losing".
	 */
	@SuppressWarnings("unchecked")
	private void spillSingleRecord(final K key, final V value)  throws IOException {
		synchronized (mergeLock) {
			long size = kvbuffer.length + partitions * APPROX_HEADER_LENGTH;
			FSDataOutputStream out = null;
			FSDataOutputStream indexOut = null;
			final int partition = partitioner.getPartition(key, value, partitions);
			try {
				// create spill file
				Path filename = mapOutputFile.getSpillFileForWrite(this.taskid, this.numSpills, size);
				out = localFs.create(filename);
				// create spill index
				Path indexFilename = mapOutputFile.getSpillIndexFileForWrite(
						this.taskid, this.numSpills,
						partitions * MAP_OUTPUT_INDEX_RECORD_LENGTH);
				indexOut = localFs.create(indexFilename);
				// we don't run the combiner for a single record
				for (int i = 0; i < partitions; ++i) {
					IFile.Writer writer = null;
					try {
						long segmentStart = out.getPos();
						// Create a new codec, don't care!
						writer = new IFile.Writer(job, out, keyClass, valClass, codec);

						if (i == partition) {
							writer.append(key, value);
						}
						writer.close();

						// index record
						writeIndexRecord(indexOut, out, segmentStart, writer);
					} catch (IOException e) {
						if (null != writer) writer.close();
						throw e;
					}
				}
				++numSpills;
			} finally {
				if (out != null) out.close();
				if (indexOut != null) indexOut.close();
			}
		}
	}

	/**
	 * Given an offset, populate vbytes with the associated set of
	 * deserialized value bytes. Should only be called during a spill.
	 */
	private void getVBytesForOffset(int kvoff, InMemValBytes vbytes) {
		final int nextindex = (kvoff / ACCTSIZE ==
			(kvend - 1 + kvoffsets.length) % kvoffsets.length)
			? bufend
					: kvindices[(kvoff + ACCTSIZE + KEYSTART) % kvindices.length];
		int vallen = (nextindex >= kvindices[kvoff + VALSTART])
		? nextindex - kvindices[kvoff + VALSTART]
		                        : (bufvoid - kvindices[kvoff + VALSTART]) + nextindex;
		vbytes.reset(kvbuffer, kvindices[kvoff + VALSTART], vallen);
	}

	@SuppressWarnings("unchecked")
	private void combineAndSpill(RawKeyValueIterator kvIter) throws IOException {
		Reducer combiner =
			(Reducer)ReflectionUtils.newInstance(combinerClass, job);
		try {
			ValuesIterator values = new ValuesIterator(
					kvIter, comparator, keyClass, valClass, job, reporter);
			while (values.more()) {
				combiner.reduce(values.getKey(), values, combineCollector, reporter);
				values.nextKey();
				// indicate we're making progress
				reporter.progress();
			}
		} finally {
			combiner.close();
		}
	}

	/**
	 * Inner class wrapping valuebytes, used for appendRaw.
	 */
	protected class InMemValBytes extends DataInputBuffer {
		private byte[] buffer;
		private int start;
		private int length;

		public void reset(byte[] buffer, int start, int length) {
			this.buffer = buffer;
			this.start = start;
			this.length = length;

			if (start + length > bufvoid) {
				this.buffer = new byte[this.length];
				final int taillen = bufvoid - start;
				System.arraycopy(buffer, start, this.buffer, 0, taillen);
				System.arraycopy(buffer, 0, this.buffer, taillen, length-taillen);
				this.start = 0;
			}

			super.reset(this.buffer, this.start, this.length);
		}
	}

	protected class MRResultIterator implements RawKeyValueIterator {
		private final DataInputBuffer keybuf = new DataInputBuffer();
		private final InMemValBytes vbytes = new InMemValBytes();
		private final int end;
		private int current;
		public MRResultIterator(int start, int end) {
			this.end = end;
			current = start - 1;
		}
		public boolean next() throws IOException {
			return ++current < end;
		}
		public DataInputBuffer getKey() throws IOException {
			final int kvoff = kvoffsets[current % kvoffsets.length];
			keybuf.reset(kvbuffer, kvindices[kvoff + KEYSTART],
					kvindices[kvoff + VALSTART] - kvindices[kvoff + KEYSTART]);
			return keybuf;
		}
		public DataInputBuffer getValue() throws IOException {
			getVBytesForOffset(kvoffsets[current % kvoffsets.length], vbytes);
			return vbytes;
		}
		public Progress getProgress() {
			return null;
		}
		public void close() { }
	}
	
	protected class FSMRResultIterator implements RawKeyValueIterator {
		private IFile.Reader reader;
		private DataInputBuffer key = new DataInputBuffer();
		private DataInputBuffer value = new DataInputBuffer();
		private Progress progress = new Progress();
		
		public FSMRResultIterator(FileSystem localFS, Path path) throws IOException {
			this.reader = new IFile.Reader<K, V>(job, localFS, path, codec);
		}

		@Override
		public void close() throws IOException {
			this.reader.close();
		}

		@Override
		public DataInputBuffer getKey() throws IOException {
			return this.key;
		}

		@Override
		public Progress getProgress() {
			try {
				float score = reader.getPosition() / (float) reader.getLength();
				progress.set(score);
			} catch (IOException e) {
				e.printStackTrace();
			}
			return progress;
		}

		@Override
		public DataInputBuffer getValue() throws IOException {
			return this.value;
		}

		@Override
		public boolean next() throws IOException {
			return this.reader.next(key, value);
		}
		
	}
	
	private synchronized int mergeParts(boolean spill) throws IOException {
		// get the approximate size of the final output/index files

		int start = 0;
		int end = 0;
		synchronized (mergeLock) {
			start = numFlush;
			end   = numSpills;

			numFlush = numSpills;
			numSpills++;
		}

			long finalOutFileSize = 0;
			long finalIndexFileSize = 0;
			Path [] filename = new Path[end];
			Path [] indexFileName = new Path[end];
			FileSystem localFs = FileSystem.getLocal(job);

			for(int i = start; i < end; i++) {
				filename[i] = mapOutputFile.getSpillFile(this.taskid, i);
				indexFileName[i] = mapOutputFile.getSpillIndexFile(this.taskid, i);
				finalOutFileSize += localFs.getFileStatus(filename[i]).getLen();
			}

			//make correction in the length to include the sequence file header
			//lengths for each partition
			finalOutFileSize += partitions * APPROX_HEADER_LENGTH;

			finalIndexFileSize = partitions * MAP_OUTPUT_INDEX_RECORD_LENGTH;

			Path outputFile = null;
			Path indexFile = null;

			if (spill) {
				outputFile = mapOutputFile.getSpillFileForWrite(this.taskid, end, finalOutFileSize);
				indexFile = mapOutputFile.getSpillIndexFileForWrite(this.taskid, end, finalIndexFileSize);
			}
			else {
				outputFile = mapOutputFile.getOutputFileForWrite(this.taskid,  finalOutFileSize);
				indexFile = mapOutputFile.getOutputIndexFileForWrite( this.taskid, finalIndexFileSize);
			}
			
			LOG.info("JBuffer " + taskid + " merge " + (end - start) + 
					 " spill files. Final? " + (!spill) + ". start = " + start + ", end = " + end + 
					 ". Output size = " + finalOutFileSize);
			if (end - start == 1) { //the spill is the final output
				localFs.rename(filename[start], outputFile); 
				localFs.rename(indexFileName[start], indexFile); 
				return end;
			}

			//The output stream for the final single output file
			FSDataOutputStream finalOut = localFs.create(outputFile, !spill);

			//The final index file output stream
			FSDataOutputStream finalIndexOut = localFs.create(indexFile, !spill);

			if (start == end) {
				//create dummy files
				if (spill) LOG.error("Error: spill file is a dummy!");
				for (int i = 0; i < partitions; i++) {
					long segmentStart = finalOut.getPos();
					IFile.Writer<K, V> writer = new IFile.Writer<K, V>(job, finalOut,  keyClass, valClass, codec);
					writer.close();
					writeIndexRecord(finalIndexOut, finalOut, segmentStart, writer);
				}
				finalOut.close();
				finalIndexOut.close();
				return -1;
			}
			{
				for (int parts = 0; parts < partitions; parts++){
					//create the segments to be merged
					BufferRequest request = requestMap.containsKey(parts) ? requestMap.get(parts) : null;

					List<Segment<K, V>> segmentList =
						new ArrayList<Segment<K, V>>(end - start);
					for(int i = start; i < end; i++) {
						if (request != null && i <= request.flushPoint()) {
							LOG.debug("JBuffer flush: pipeline request already took care of partition " 
									  + parts + " spill file " + i);
							continue; // Request has already sent this spill data.
						}

						FSDataInputStream indexIn = localFs.open(indexFileName[i]);
						indexIn.seek(parts * MAP_OUTPUT_INDEX_RECORD_LENGTH);
						long segmentOffset = indexIn.readLong();
						long rawSegmentLength = indexIn.readLong();
						long segmentLength = indexIn.readLong();
						indexIn.close();
						FSDataInputStream in = localFs.open(filename[i]);
						in.seek(segmentOffset);
						Segment<K, V> s = 
							new Segment<K, V>(new IFile.Reader<K, V>(job, in, segmentLength, codec),
									true);
						segmentList.add(s);
					}

					//merge
					@SuppressWarnings("unchecked")
					RawKeyValueIterator kvIter = 
						Merger.merge(job, localFs, 
								keyClass, valClass,
								segmentList, job.getInt("io.sort.factor", 100), 
								new Path(this.taskid.toString()), 
								job.getOutputKeyComparator(), reporter);

					//write merged output to disk
					long segmentStart = finalOut.getPos();
					IFile.Writer<K, V> writer = 
						new IFile.Writer<K, V>(job, finalOut, keyClass, valClass, codec);
					if (null == combinerClass || end - start < minSpillsForCombine) {
						Merger.writeFile(kvIter, writer, reporter, job);
					} else {
						combineCollector.setWriter(writer);
						combineAndSpill(kvIter);
					}

					//close
					writer.close();
					
					//write index record
					writeIndexRecord(finalIndexOut, finalOut, segmentStart, writer);
				}
				finalOut.close();
				finalIndexOut.close();
				//cleanup
				for(int i = start; i < end; i++) {
					localFs.delete(filename[i], true);
					localFs.delete(indexFileName[i], true);
					filename[i] = indexFileName[i] = null;
				}
			}

		return spill ? end : -1;
	}

	private void writeIndexRecord(FSDataOutputStream indexOut, 
			FSDataOutputStream out, long start, 
			IFile.Writer<K, V> writer) 
	throws IOException {
		//when we write the offset/decompressed-length/compressed-length to  
		//the final index file, we write longs for both compressed and 
		//decompressed lengths. This helps us to reliably seek directly to 
		//the offset/length for a partition when we start serving the 
		//byte-ranges to the reduces. We probably waste some space in the 
		//file by doing this as opposed to writing VLong but it helps us later on.
		// index record: <offset, raw-length, compressed-length> 
		//StringBuffer sb = new StringBuffer();
		indexOut.writeLong(start);
		indexOut.writeLong(writer.getRawLength());
		long segmentLength = out.getPos() - start;
		indexOut.writeLong(segmentLength);
	}

	/**
	 * Exception indicating that the allocated sort buffer is insufficient
	 * to hold the current record.
	 */
	@SuppressWarnings("serial")
	private static class MapBufferTooSmallException extends IOException {
		public MapBufferTooSmallException(String s) {
			super(s);
		}
	}
} // MapOutputBuffer
