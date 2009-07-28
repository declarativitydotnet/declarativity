package org.apache.hadoop.mapred;

import static org.apache.hadoop.mapred.Task.Counter.COMBINE_INPUT_RECORDS;
import static org.apache.hadoop.mapred.Task.Counter.COMBINE_OUTPUT_RECORDS;
import static org.apache.hadoop.mapred.Task.Counter.MAP_OUTPUT_BYTES;
import static org.apache.hadoop.mapred.Task.Counter.MAP_OUTPUT_RECORDS;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.io.serializer.SerializationFactory;
import org.apache.hadoop.io.serializer.Serializer;
import org.apache.hadoop.mapred.IFile.Writer;
import org.apache.hadoop.mapred.MapTask.MapOutputCollector;
import org.apache.hadoop.mapred.Task.CombineOutputCollector;
import org.apache.hadoop.mapred.Task.CombineValuesIterator;
import org.apache.hadoop.util.IndexedSortable;
import org.apache.hadoop.util.IndexedSorter;
import org.apache.hadoop.util.Progress;
import org.apache.hadoop.util.QuickSort;
import org.apache.hadoop.util.ReflectionUtils;

public class MapOutputPipe<K extends Object, V extends Object> 
implements MapOutputCollector<K, V>, IndexedSortable {
	private static final Log LOG = LogFactory.getLog(MapOutputPipe.class.getName());

	/** The size of each record in the index file for the map-outputs. */
	public static final int MAP_OUTPUT_INDEX_RECORD_LENGTH = 24;
	private final static int APPROX_HEADER_LENGTH = 150;

	private final MapTask task;
	
	private final int partitions;
	private final Partitioner<K, V> partitioner;
	private final JobConf job;
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
	
	private Writer<K,V>[] pipes;

	// spill accounting
	private volatile int numSpills = 0;
	private volatile Throwable sortSpillException = null;
	private final int softRecordLimit;
	private final int softBufferLimit;
	private final int minSpillsForCombine;
	private final IndexedSorter sorter;
	private final Object spillLock = new Object();
	private final BlockingBuffer bb = new BlockingBuffer();

	private final Counters.Counter mapOutputByteCounter;
	private final Counters.Counter mapOutputRecordCounter;
	private final Counters.Counter combineInputCounter;
	private final Counters.Counter combineOutputCounter;

	private final TaskUmbilicalProtocol umbilical;

	@SuppressWarnings("unchecked")
	public MapOutputPipe(TaskUmbilicalProtocol umbilical, JobConf job, Reporter reporter, MapTask task) throws IOException {
		this.task = task;
		this.job = job;
		this.reporter = reporter;
		this.umbilical = umbilical;
		this.pipes = null;

		partitions = job.getNumReduceTasks();
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
		LOG.info("io.sort.mb = " + sortmb);
		// buffers and accounting
		int maxMemUsage = sortmb << 20;
		int recordCapacity = (int)(maxMemUsage * recper);
		recordCapacity -= recordCapacity % RECSIZE;
		kvbuffer = new byte[maxMemUsage - recordCapacity];
		bufvoid = kvbuffer.length;
		recordCapacity /= RECSIZE;
		kvoffsets = new int[recordCapacity];
		kvindices = new int[recordCapacity * ACCTSIZE];
		softBufferLimit = (int)(kvbuffer.length * spillper);
		softRecordLimit = (int)(kvoffsets.length * spillper);
		LOG.info("data buffer = " + softBufferLimit + "/" + kvbuffer.length);
		LOG.info("record buffer = " + softRecordLimit + "/" + kvoffsets.length);
		// k/v serialization
		comparator = job.getOutputKeyComparator();
		keyClass = (Class<K>)job.getMapOutputKeyClass();
		valClass = (Class<V>)job.getMapOutputValueClass();
		serializationFactory = new SerializationFactory(job);
		keySerializer = serializationFactory.getSerializer(keyClass);
		keySerializer.open(bb);
		valSerializer = serializationFactory.getSerializer(valClass);
		valSerializer.open(bb);
		// counters
		Counters counters = task.getCounters();
		mapOutputByteCounter = counters.findCounter(MAP_OUTPUT_BYTES);
		mapOutputRecordCounter = counters.findCounter(MAP_OUTPUT_RECORDS);
		combineInputCounter = counters.findCounter(COMBINE_INPUT_RECORDS);
		combineOutputCounter = counters.findCounter(COMBINE_OUTPUT_RECORDS);
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
		? new CombineOutputCollector(combineOutputCounter)
		: null;
		minSpillsForCombine = job.getInt("min.num.spills.for.combine", 3);
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
			mapOutputByteCounter.increment(valend >= keystart
					? valend - keystart
							: (bufvoid - keystart) + valend);

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
			mapOutputRecordCounter.increment(1);

			// update accounting info
			int ind = kvindex * ACCTSIZE;
			kvoffsets[kvindex] = ind;
			kvindices[ind + PARTITION] = partition;
			kvindices[ind + KEYSTART] = keystart;
			kvindices[ind + VALSTART] = valstart;
			kvindex = (kvindex + 1) % kvoffsets.length;
		} catch (MapBufferTooSmallException e) {
			LOG.info("Record too large for in-memory buffer: " + e.getMessage());
			spillSingleRecord(key, value);
			mapOutputRecordCounter.increment(1);
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
								LOG.info("Spilling map output: buffer full = " + bufsoftlimit+
										" and record full = " + kvsoftlimit);
								LOG.info("bufstart = " + bufstart + "; bufend = " + bufmark +
										"; bufvoid = " + bufvoid);
								LOG.info("kvstart = " + kvstart + "; kvend = " + kvindex +
										"; length = " + kvoffsets.length);
								kvend = kvindex;
								bufend = bufmark;
								// TODO No need to recreate this thread every time
								SpillThread t = new SpillThread();
								t.setDaemon(true);
								t.setName("SpillThread");
								t.start();
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

	public synchronized void flush() throws IOException {
		LOG.info("Starting flush of map output");
		synchronized (spillLock) {
			while (kvstart != kvend) {
				try {
					reporter.progress();
					spillLock.wait();
				} catch (InterruptedException e) {
					throw (IOException)new IOException(
							"Buffer interrupted while waiting for the writer"
					).initCause(e);
				}
			}
		}
		if (sortSpillException != null) {
			throw (IOException)new IOException("Spill failed"
			).initCause(sortSpillException);
		}
		if (kvend != kvindex) {
			LOG.info("bufstart = " + bufstart + "; bufend = " + bufmark +
					"; bufvoid = " + bufvoid);
			LOG.info("kvstart = " + kvstart + "; kvend = " + kvindex +
					"; length = " + kvoffsets.length);
			kvend = kvindex;
			bufend = bufmark;
			flushBuffer();
		}
		kvbuffer = null;
	}

	public void close() { try {
		closePipes();
	} catch (IOException e) {
	} }

	protected class SpillThread extends Thread {
		@Override
		public void run() {
			try {
				flushBuffer();
			} catch (Throwable e) {
				sortSpillException = e;
			} finally {
				synchronized(spillLock) {
					if (bufend < bufindex && bufindex < bufstart) {
						bufvoid = kvbuffer.length;
					}
					kvstart = kvend;
					bufstart = bufend;
					spillLock.notify();
				}
			}
		}
	}
	
	private void openPipes() throws IOException {
		if (pipes == null) {
			ReduceScheduleEvent[] reduceLocations = null; // umbilical.getReduceEvents(task.getJobID());
			if (reduceLocations == null) return;
			else if (reduceLocations.length != partitions) {
				LOG.info("Do not have all reduce schedule events!");
				return;
			}
			
			pipes = new Writer[partitions];
			for (ReduceScheduleEvent reduce : reduceLocations) {
				Socket socket = new Socket();
				socket.connect(reduce.getSocketAddress());
				FSDataOutputStream out = new FSDataOutputStream(socket.getOutputStream());
				pipes[reduce.getPartition()] = new Writer<K, V>(job, out, keyClass, valClass, codec);
			}
		}
	}
	
	private void closePipes() throws IOException {
		if (pipes != null) {
			for (Writer pipe : pipes) {
				pipe.close();
			}
		}
	}

	private void flushBuffer() throws IOException {
		openPipes();
		if (pipes == null) return;
		
		//approximate the length of the output file to be the length of the
		//buffer + header lengths for the partitions
		long size = (bufend >= bufstart
				? bufend - bufstart
						: (bufvoid - bufend) + bufstart) +
						partitions * APPROX_HEADER_LENGTH;
		final int endPosition = (kvend > kvstart)
		? kvend
				: kvoffsets.length + kvend;
		sorter.sort(MapOutputPipe.this, kvstart, endPosition, reporter);
		int spindex = kvstart;
		InMemValBytes value = new InMemValBytes();
		for (int i = 0; i < partitions; ++i) {
			IFile.Writer<K, V> writer = pipes[i];
			if (null == combinerClass) {
				// spill directly
				DataInputBuffer key = new DataInputBuffer();
				while (spindex < endPosition &&
						kvindices[kvoffsets[spindex % kvoffsets.length]
						                    + PARTITION] == i) {
					final int kvoff = kvoffsets[spindex % kvoffsets.length];
					getVBytesForOffset(kvoff, value);
					key.reset(kvbuffer, kvindices[kvoff + KEYSTART],
							(kvindices[kvoff + VALSTART] - 
									kvindices[kvoff + KEYSTART]));
					writer.append(key, value);
					++spindex;
				}
			} else {
				int spstart = spindex;
				while (spindex < endPosition &&
						kvindices[kvoffsets[spindex % kvoffsets.length]
						                    + PARTITION] == i) {
					++spindex;
				}
				// Note: we would like to avoid the combiner if we've fewer
				// than some threshold of records for a partition
				if (spstart != spindex) {
					combineCollector.setWriter(writer);
					RawKeyValueIterator kvIter = new MRResultIterator(spstart, spindex);
					combineAndSpill(kvIter, combineInputCounter);
				}
			}
		}
		LOG.info("Finished spill " + numSpills);
		++numSpills;
	}

	/**
	 * Handles the degenerate case where serialization fails to fit in
	 * the in-memory buffer, so we must spill the record from collect
	 * directly to a spill file. Consider this "losing".
	 */
	@SuppressWarnings("unchecked")
	private void spillSingleRecord(final K key, final V value)  throws IOException {
		openPipes();
		int partition = partitioner.getPartition(key, value, partitions);
		// create spill file
		IFile.Writer writer = pipes[partition];
		writer.append(key, value);
		++numSpills;
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
	private void combineAndSpill(RawKeyValueIterator kvIter,
			Counters.Counter inCounter) throws IOException {
		Reducer combiner =
			(Reducer)ReflectionUtils.newInstance(combinerClass, job);
		try {
			CombineValuesIterator values = new CombineValuesIterator(
					kvIter, comparator, keyClass, valClass, job, reporter,
					inCounter);
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


	public class MRResultIterator implements RawKeyValueIterator {
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

}
