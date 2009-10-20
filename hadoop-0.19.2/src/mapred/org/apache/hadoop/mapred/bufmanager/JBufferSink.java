package org.apache.hadoop.mapred.bufmanager;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.ChecksumException;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.mapred.IFile;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.FileHandle;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.Task;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.util.ReflectionUtils;


public class JBufferSink<K extends Object, V extends Object> {
	private static final Log LOG = LogFactory.getLog(JBufferSink.class.getName());
	  
	public class JBufferSnapshot {
		public boolean fresh = false;
		
		private TaskID taskid;
		
		private Path data  = null;
		private Path index = null;
		
		private long length = 0;
		
		private float progress = 0f;
		
		private int runs = 0;
		
		public JBufferSnapshot(TaskID taskid) {
			this.taskid = taskid;
		}
		
		public String toString() {
			return "JBufferSnapshot " + taskid + ": progress = " + progress;
		}
		
		public boolean valid() {
			return this.data != null && length > 0;
		}
		
		/**
		 * Spill the current snapshot to the buffer.
		 * This method ensures that a consistent snapshot is spilled to
		 * the buffer. We don't want the next snapshot interfering with
		 * the buffer spill.
		 * @param buffer
		 * @throws IOException
		 */
		public void spill(JBufferCollector buffer) throws IOException {
			synchronized (this) {
				if (!valid()) {
					throw new IOException("JBufferRun not valid!");
				}
				buffer.spill(data, index, JBufferCollector.SpillOp.COPY);
				fresh = false;
			}
		}
		
		/**
		 * Create a new snapshot.
		 * @param reader Input data to be sent to the new snapshot.
		 * @param length Length of the data.
		 * @param progress Progress of this snapshot.
		 * @throws IOException
		 */
		public void 
		snapshot(IFile.Reader<K, V> reader, long length, float progress) 
		throws IOException {
			synchronized (this) {
				if (this.progress < progress) {
					runs++;
					Path data = fileHandle.getInputSnapshotFileForWrite(ownerid, taskid, runs, length);
					Path index = fileHandle.getInputSnapshotIndexFileForWrite(ownerid, taskid, runs, 1096);
					FSDataOutputStream out  = localFs.create(data, false);
					FSDataOutputStream idx = localFs.create(index, false);
					if (out == null) throw new IOException("Unable to create snapshot " + data);
					write(reader, out, idx);
					this.length   = length;
					this.progress = progress;
					this.data     = data;
					this.index    = index;
					this.fresh    = true;
				}
			}
			snapshotThread.snapshot();
		}
		
		private void write(IFile.Reader<K, V> in, FSDataOutputStream out, FSDataOutputStream index) throws IOException {
			Class <K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
			Class <V> valClass = (Class<V>)conf.getMapOutputValueClass();
			CompressionCodec codec = null;
			if (conf.getCompressMapOutput()) {
				Class<? extends CompressionCodec> codecClass =
					conf.getMapOutputCompressorClass(DefaultCodec.class);
				codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
			}

			DataInputBuffer key = new DataInputBuffer();
			DataInputBuffer value = new DataInputBuffer();
			IFile.Writer<K, V> writer = new IFile.Writer<K, V>(conf, out,  keyClass, valClass, codec);
			/* Copy over the data until done. */
			while (in.next(key, value)) {
				writer.append(key, value);
			}
			writer.close();
			out.close();
			
			/* Write the index file. */
			index.writeLong(0);
			index.writeLong(writer.getRawLength());
			index.writeLong(out.getPos());

			/* Close everything. */
			index.flush();
			index.close();
		}
	}
	
	public class SnapshotThread extends Thread {
		private boolean busy = false;
		private boolean open = true;
		
		public void close() {
			if (!open) return;
			else open = false;
			
			synchronized (this) {
				this.notifyAll();
				while (busy) {
					try {
						this.wait();
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
		}
		
		public void snapshot() {
			synchronized (this) {
				this.notifyAll();
			}
		}
		
		public void run() {
			float progress = 0f;
			try {
				List<JBufferSink.JBufferSnapshot> snapshots = new ArrayList<JBufferSink.JBufferSnapshot>();
				while (open) {
					synchronized (this) {
						busy = false;
						this.notifyAll();

						int freshRuns = 0;
						do {
							if (!open) return;
							try { this.wait();
							} catch (InterruptedException e) { return; }
							if (!open) return;

							freshRuns = 0;
							for (JBufferSnapshot snapshot : inputSnapshots.values()) {
								if (snapshot.fresh) freshRuns++;
							}
						} while (freshRuns < inputSnapshots.size() / 3);
						busy = true;
					}

					synchronized (inputSnapshots) {
						snapshots.clear();
						progress = 0f;
						for (JBufferSnapshot snapshot : inputSnapshots.values()) {
							if (snapshot.valid()) {
								progress += snapshot.progress;
								snapshots.add(snapshot);
							}
						}
						progress = progress / (float) numInputs;
					}

					try {
						if (!open) return;
						LOG.debug("SnapshotThread: " + ownerid + " perform snapshot. progress " + progress);
						boolean keepSnapshotting = task.snapshots(snapshots, progress);
						if (keepSnapshotting == false) {
							interrupt();
							return;
						}
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
			finally {
				synchronized (this) {
					open = false;
					busy = false;
					this.notifyAll();
				}
			}
		}
	}
	
	private class SpillRun {
		TaskID taskid;
		
		float progress;
		
		Path data;
		
		Path index;
		
		public SpillRun(TaskID taskid, float progress, Path data, Path index) {
			this.taskid = taskid;
			this.progress = progress;
			this.data = data;
			this.index = index;
		}
		
	}
	
	private Reporter reporter;
	
	private JobConf conf;
	
	private TaskAttemptID ownerid;
	
	private int maxConnections;
	
	private Executor executor;
	
	private FileSystem localFs;
	
	private Thread acceptor;
	
	private ServerSocketChannel server;
	
	private int numInputs;
	
	private JBufferCollector<K, V> collector;
	
	private Set<Connection> connections;
	
	private Map<TaskID, Float> inputProgress;
	
	private Map<TaskID, JBufferSnapshot> inputSnapshots;
	
	private Thread spillThread;
	
	private BlockingQueue<SpillRun> spillQueue;
	
	private Set<TaskID> successful;
	
	private Task task;
	
	private SnapshotThread snapshotThread;
	
	private FileHandle fileHandle;
	
	public JBufferSink(JobConf conf, Reporter reporter, JBufferCollector<K, V> collector, Task task) throws IOException {
		this.conf = conf;
		this.reporter = reporter;
		this.ownerid = task.getTaskID();
		this.collector = collector;
		this.localFs = FileSystem.getLocal(conf);
		this.maxConnections = conf.getInt("mapred.reduce.parallel.copies", 20);
		this.fileHandle = new FileHandle(ownerid.getJobID());
		this.fileHandle.setConf(conf);
		this.inputSnapshots = new HashMap<TaskID, JBufferSnapshot>();
		this.spillQueue = new LinkedBlockingQueue<SpillRun>();
		
		this.task = task;
	    this.numInputs = task.getNumberOfInputs();
		this.executor = Executors.newFixedThreadPool(Math.min(maxConnections, Math.max(numInputs, 5)));
		this.connections = new HashSet<Connection>();
		this.successful = new HashSet<TaskID>();
		this.inputProgress = new HashMap<TaskID, Float>();
		
		this.snapshotThread = null;
		this.snapshotThread = new SnapshotThread();
		this.snapshotThread.setDaemon(true);
		this.snapshotThread.start();
		
		/** The server socket and selector registration */
		this.server = ServerSocketChannel.open();
		this.server.configureBlocking(true);
		this.server.socket().bind(new InetSocketAddress(0));
	}
	
	public InetSocketAddress getAddress() {
		try {
			String host = InetAddress.getLocalHost().getCanonicalHostName();
			return new InetSocketAddress(host, this.server.socket().getLocalPort());
		} catch (UnknownHostException e) {
			return new InetSocketAddress("localhost", this.server.socket().getLocalPort());
		}
	}
	
	public void open() {
		this.acceptor = new Thread() {
			public void run() {
				try {
					BufferRequestResponse response = new BufferRequestResponse();
					while (server.isOpen()) {
						SocketChannel channel = server.accept();
						channel.configureBlocking(true);
						DataInputStream  input  = new DataInputStream(new BufferedInputStream(channel.socket().getInputStream()));
						Connection       conn   = new Connection(input, conf);

						DataOutputStream output = new DataOutputStream(new BufferedOutputStream(channel.socket().getOutputStream()));
						response.reset();
						LOG.debug("JBufferSink " + ownerid + " receive connection " + channel.socket().getRemoteSocketAddress());

						if (complete()) {
							LOG.debug("JBufferSink: " + ownerid + " is complete. Terminating " + channel.socket().getRemoteSocketAddress());
							response.setTerminated();
							response.write(output);
							output.flush();
							conn.close();
						}
						else if (connections.size() > maxConnections) {
							LOG.debug("JBufferSink: " + ownerid + " max connections reached. Terminating " + channel.socket().getRemoteSocketAddress());
							response.setRetry();
							response.write(output);
							output.flush();
							conn.close();
						}
						else {
							try {
								response.setOpen();
								response.write(output);
								output.flush();
								executor.execute(conn);
								connections.add(conn);
								LOG.debug("JBufferSink: " + ownerid + " accepted connection " + channel.socket().getRemoteSocketAddress());
							} catch (Throwable t) {
								LOG.warn("Received error when trying to execute connection. " + t);
								conn.close();
							}
						}
					}
					LOG.info("JBufferSink " + ownerid + " buffer response server closed.");
				} catch (IOException e) {  }
			}
		};
		acceptor.setDaemon(true);
		acceptor.setPriority(Thread.MAX_PRIORITY);
		acceptor.start();
		
		spillThread = new Thread() {
			@Override
			public void interrupt() {
				while (spillQueue.size() > 0) {
					drain();
				}
				super.interrupt();
			}
			
			private void drain() {
				synchronized (task) {
					List<SpillRun> runs = new ArrayList<SpillRun>();
					spillQueue.drainTo(runs);
					long timestamp = System.currentTimeMillis();
					LOG.debug("JBufferSink begin drain spill runs.");
					for (SpillRun run : runs) {
						try { 
							if (getProgress(run.taskid) < run.progress) {
								collector.spill(run.data, run.index, JBufferCollector.SpillOp.COPY);
								updateProgress(run.taskid, run.progress);
							}
						} catch (IOException e) {
							e.printStackTrace();
						}
					}
					LOG.debug("JBufferSink end drain spills. total time = " + 
							  (System.currentTimeMillis() - timestamp) + " ms.");
				}
			}
			
			public void run() {
				try {
					while (!isInterrupted()) {
						try {
							SpillRun run = spillQueue.take();
							spillQueue.add(run);
							drain();
						} catch (InterruptedException e) {
							return;
						}
					}
				} finally {
					LOG.info("JBufferSink " + ownerid + " spill thread exit.");
				}
			}
		};
		spillThread.setDaemon(true);
		spillThread.start();
	}
	
	public TaskAttemptID ownerID() {
		return this.ownerid;
	}
	
	/**
	 * Close sink.
	 * No more connections can be made once closed. Method will
	 * lock the owning task object if snapshots are turned on.
	 * @return true if the complete snapshot of all input snapshots were
	 * applied to the buffer.
	 */
	public synchronized void close() {
		LOG.info("JBufferSink is closing.");
		if (this.acceptor == null) return; // Already done.
		try {
			this.acceptor.interrupt();
			this.server.close();
			this.acceptor = null;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		this.spillThread.interrupt();

		try {
			this.snapshotThread.close();
			LOG.info("JBufferSink " + ownerid + " snapshot thread closed.");
			synchronized (inputSnapshots) {
				if (inputSnapshots.size() == successful.size()) {
					synchronized (task) {
						for (JBufferSnapshot snapshot : inputSnapshots.values()) {
							LOG.info("JBufferSink " + ownerid + " apply run " + snapshot.taskid);
							if (snapshot.progress < 1.0f) {
								LOG.error("JBufferSink: final buffer run progress < 1.0f! " +
										" progress = " + snapshot.progress);
							}
							snapshot.spill(this.collector);
						}
					}
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	private void spill(TaskID taskid, float progress, Path data, Path index) {
		spillQueue.add(new SpillRun(taskid, progress, data, index));
	}
	
	private JBufferCollector<K, V> buffer() {
		return this.collector;
	}
	
	public boolean complete() {
		return this.successful.size() == numInputs;
	}
	
	public void cancel(TaskAttemptID attempt) {
		// TODO 
	}
	
	private void done(Connection connection) {
		this.connections.remove(connection);
	}
	
	private float getProgress(TaskID taskid) {
		return this.inputProgress.containsKey(taskid) ? 
				this.inputProgress.get(taskid) : 0f;
	}
	
	private void updateProgress(TaskID taskid, float progress) {
		if (progress == 1f) {
			this.successful.add(taskid);
		}
		if (!this.inputProgress.containsKey(taskid) ||
				this.inputProgress.get(taskid) < progress) {
			this.inputProgress.put(taskid, progress);
			float progressSum = 0f;
			for (Float f : this.inputProgress.values()) {
				progressSum += f;
			}
			
			collector.getProgress().set(progressSum / (float) numInputs);
			reporter.progress();
		}
	}
	
	private JBufferSnapshot getBufferRun(TaskID taskid) {
		synchronized (inputSnapshots) {
			if (!this.inputSnapshots.containsKey(taskid)) {
				this.inputSnapshots.put(taskid, new JBufferSnapshot(taskid));
			}
			return this.inputSnapshots.get(taskid);
		}
	}
	
	/************************************** CONNECTION CLASS **************************************/
	
	private class Connection implements Runnable {
		private DataInputStream input;
		
		private boolean open;
		
		private int spills;
		
		public Connection(DataInputStream input, JobConf conf) throws IOException {
			this.input = input;
			this.open = true;
			this.spills = 0;
		}
		
		public void close() {
			synchronized (this) {
				open = false;
				if (this.input == null) return;
				try {
					this.input.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
				finally {
					this.input = null;
				}
			}
		}
		
		private void 
		spill(TaskID taskid, float progress, IFile.Reader<K, V> reader, long length, Class<K> keyClass, Class<V> valClass, CompressionCodec codec) 
		throws IOException {
			// Spill directory to disk
			int spillid = spills++;
			Path filename      = fileHandle.getInputSpillFileForWrite(ownerid, taskid, spillid, length);
			Path indexFilename = fileHandle.getInputSpillIndexFileForWrite(ownerid, taskid, spillid, JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);

			FSDataOutputStream out      = localFs.create(filename, false);
			FSDataOutputStream indexOut = localFs.create(indexFilename, false);

			if (out == null || indexOut == null) 
				throw new IOException("Unable to create spill file " + filename);

			IFile.Writer<K, V> writer = new IFile.Writer<K, V>(conf, out,  keyClass, valClass, codec);
			DataInputBuffer key = new DataInputBuffer();
			DataInputBuffer value = new DataInputBuffer();
			try {
				/* Copy over the data until done. */
				while (reader.next(key, value)) {
					writer.append(key, value);
				}
				writer.close();
				out.close();

				/* Write the index file. */
				indexOut.writeLong(0);
				indexOut.writeLong(writer.getRawLength());
				indexOut.writeLong(out.getPos());

				/* Close everything. */
				indexOut.flush();
				indexOut.close();

				JBufferSink.this.spill(taskid, progress, filename, indexFilename);
			} catch (Throwable e) {
				LOG.error("JBufferSink: error " + e + " during spill. progress = " + progress);
				e.printStackTrace();
			}
		}
		
		private void service(OutputFile.Header header, long length) throws IOException {
			DataInputBuffer key = new DataInputBuffer();
			DataInputBuffer value = new DataInputBuffer();
			
			Class <K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
			Class <V> valClass = (Class<V>)conf.getMapOutputValueClass();
			
			CompressionCodec codec = null;
			if (conf.getCompressMapOutput()) {
				Class<? extends CompressionCodec> codecClass =
					conf.getMapOutputCompressorClass(DefaultCodec.class);
				codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
			}
			
			IFile.Reader<K, V> reader = 
				new IFile.Reader<K, V>(conf, input, length, codec);
			
			if (header.type() == OutputFile.Type.SNAPSHOT) {
				try {
					LOG.debug("JBufferSink: perform snaphot to buffer " + ownerid + " from buffer " + 
							header.owner() + " progress = " + header.progress());
					JBufferSnapshot snapshot = getBufferRun(header.owner().getTaskID());
					snapshot.snapshot(reader, length, header.progress());
				} catch (Throwable t) {
					t.printStackTrace();
					LOG.warn("Snapshot interrupted by " + t);
				}
			} else {
				if (task.isSnapshotting() || task.isMerging()) {
					/* Drain socket while task is snapshotting. */
					LOG.debug("JBufferSink: call spill for data " + header.owner() + " due to merging or snapshotting.");
					spill(header.owner().getTaskID(), header.progress(), reader, length, keyClass, valClass, codec);
				} else { 
					boolean doSpill = true;
						LOG.debug("JBufferSink: get task lock for " + header.owner() + " dump.");
						synchronized (task) {
							LOG.debug("JBufferSink: try to reserve " + length + " buffer space for buffer " + header.owner());
							/* Try to add records to the buffer. 
							 * Note: this means we can't back out the records so
							 * if we're in safemode this needs to be the final answer.
							 */
							if (buffer().reserve(length)) {
								LOG.debug("JBufferSink: dumping buffer " + header.owner() + ".");
								int records = 0;
								try {
									while (reader.next(key, value)) {
										records++;
										buffer().collect(key, value);
									}
								} catch (ChecksumException e) {
									e.printStackTrace();
								} catch (Throwable t) {
									t.printStackTrace();
								} finally {
									LOG.debug("JBufferSink: dumped " + records + " records.");
									buffer().unreserve(length);
									doSpill = false;
									updateProgress(header.owner().getTaskID(), header.progress());
									task.notifyAll();
								}
							}
							else {
								LOG.debug("JBufferSink: unable to reserve buffer space for " + header.owner());
							}
						}
					
					if (doSpill) {
						LOG.debug("JBufferSink: had to spill " + header.owner() + ".");
						spill(header.owner().getTaskID(), header.progress(), reader, length, keyClass, valClass, codec);
					}
				}
			}
		}
		
		public void run() {
			try {
				while (open) {
					long length = 0;
					OutputFile.Header header = null;
					try {
						length = this.input.readLong();
						header = OutputFile.Header.readHeader(this.input);
					}
					catch (Throwable e) {
						return;
					}
					
					if (length == 0) {
						updateProgress(header.owner().getTaskID(), header.progress());
					}
					else {
						long timestamp = System.currentTimeMillis();
						LOG.debug("JBufferSink receiving data from task " + header.owner() + 
								" service length " +  length + " progress = " + header.progress());
						service(header, length);
						LOG.debug("JBufferSink data from task " + header.owner() + " service time = " + 
								  (System.currentTimeMillis() - timestamp));
					}
				}
			} catch (Throwable e) {
				e.printStackTrace();
				return;
			}
			finally {
				close(); // must be called before done!
				done(this);
			}
		}
	}
}
