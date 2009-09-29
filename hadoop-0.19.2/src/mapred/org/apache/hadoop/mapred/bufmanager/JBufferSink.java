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
import org.apache.hadoop.mapred.ReduceOutputFile;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.Task;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.util.ReflectionUtils;


public class JBufferSink<K extends Object, V extends Object> {
	private static final Log LOG = LogFactory.getLog(JBufferSink.class.getName());
	  
	public class JBufferRun {
		public boolean fresh = false;
		
		private TaskID id;
		
		private Path snapshot = null;
		private Path index    = null;
		
		private long length = 0;
		
		private float progress = 0f;
		
		private int runs = 0;
		
		public JBufferRun(TaskID id) {
			this.id = id;
		}
		
		public String toString() {
			return "JBufferRun " + id + ": progress = " + progress;
		}
		
		public boolean valid() {
			return this.snapshot != null && length > 0;
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
				buffer.spill(snapshot, index, true);
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
				runs++;
				Path data = outputFileManager.getOutputRunFileForWrite(id, runs, 1096);
				Path index = outputFileManager.getOutputRunIndexFileForWrite(id, runs, 1096);
				FSDataOutputStream out  = localFs.create(data, false);
				FSDataOutputStream idx = localFs.create(index, false);
				if (out == null) throw new IOException("Unable to create snapshot " + data);
				write(reader, out, idx);
				this.length   = length;
				this.progress = progress;
				this.snapshot = data;
				this.index    = index;
				this.fresh    = true;
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
				List<JBufferSink.JBufferRun> runs = new ArrayList<JBufferSink.JBufferRun>();
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
							for (JBufferRun run : bufferRuns.values()) {
								if (run.fresh) freshRuns++;
							}
						} while (freshRuns < bufferRuns.size() / 3);
						busy = true;
					}

					synchronized (bufferRuns) {
						runs.clear();
						progress = 0f;
						for (JBufferRun run : bufferRuns.values()) {
							if (run.valid()) {
								progress += run.progress;
								runs.add(run);
							}
						}
						progress = progress / (float) numConnections;
					}

					try {
						if (!open) return;
						LOG.debug("SnapshotThread: " + reduceID + " perform snapshot. progress " + progress);
						boolean keepSnapshotting = task.snapshots(runs, progress);
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
		Path data;
		
		Path index;
		
		public SpillRun(Path data, Path index) {
			this.data = data;
			this.index = index;
		}
		
	}
	
	private Reporter reporter;
	
	private boolean safemode;
	
	private int maxConnections;
	
	private Executor executor;
	
	private FileSystem localFs;
	
	private Thread acceptor;
	
	private ServerSocketChannel server;
	
	private int numConnections;
	
	private JBufferCollector<K, V> collector;
	
	private JobConf conf;
	
	private TaskAttemptID reduceID;
	
	private Map<TaskID, List<Connection>> connections;
	
	private Map<TaskID, JBufferRun> bufferRuns;
	
	private Thread spillThread;
	
	private BlockingQueue<SpillRun> spillQueue;
	
	private int spillCount;
	
	private Set<TaskID> successful;
	
	private Task task;
	
	private SnapshotThread snapshotThread;
	
	private final boolean snapshots;
	
	private ReduceOutputFile outputFileManager;
	
	public JBufferSink(JobConf conf, Reporter reporter, TaskAttemptID reduceID, JBufferCollector<K, V> collector, Task task, boolean snapshots) throws IOException {
		this.conf = conf;
		this.reporter = reporter;
		this.safemode = conf.getBoolean("mapred.safemode", false);
		this.reduceID = reduceID;
		this.collector = collector;
		this.localFs = FileSystem.getLocal(conf);
		this.maxConnections = conf.getInt("mapred.reduce.parallel.copies", 20);
		this.outputFileManager = new ReduceOutputFile(reduceID);
		this.outputFileManager.setConf(conf);
		this.bufferRuns = new HashMap<TaskID, JBufferRun>();
		this.spillQueue = new LinkedBlockingQueue<SpillRun>();
		this.spillCount = 0;
		
		this.task = task;
	    this.numConnections = task.getNumberOfInputs();
		this.executor = Executors.newCachedThreadPool();
		this.connections = new ConcurrentHashMap<TaskID, List<Connection>>();
		this.successful = new HashSet<TaskID>();
		
		this.snapshots = snapshots;
		this.snapshotThread = null;
		if (snapshots) {
			this.snapshotThread = new SnapshotThread();
			this.snapshotThread.setDaemon(true);
			this.snapshotThread.start();
		}
		
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
	
	public boolean snapshots() {
		return this.snapshots;
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
						Connection       conn   = new Connection(input, JBufferSink.this, conf);
						
							TaskID taskid = conn.id().getTaskID();
							DataOutputStream output = new DataOutputStream(new BufferedOutputStream(channel.socket().getOutputStream()));
							
							if (complete() || successful.contains(taskid) ) {
								response.setTerminated();
								response.write(output);
								output.flush();
								conn.close();
							}
							else if (!connections.containsKey(taskid) &&
									  (connections.size() - successful.size()) > maxConnections) {
								response.setRetry();
								response.write(output);
								output.flush();
								conn.close();
							}
							else {
								if (!conn.isSnapshot()) {
									/* register regular connection. */
									synchronized (connections) {
										if (!connections.containsKey(taskid)) {
											connections.put(taskid, new ArrayList<Connection>());
										}
										connections.get(taskid).add(conn);
									}
								}
								try {
									response.setOpen();
									response.write(output);
									output.flush();
									
									executor.execute(conn);
								} catch (Throwable t) {
									LOG.warn("Received error when trying to execute connection. " + t);
									synchronized (connections) {
										connections.get(taskid).remove(conn);
									}
									response.setRetry();
									response.write(output);
									output.flush();
									conn.close();
								}
							}
					}
					LOG.info("JBufferSink " + reduceID + " buffer response server closed.");
				} catch (IOException e) {  }
			}
		};
		acceptor.setDaemon(true);
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
							collector.spill(run.data, run.index, false);
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
							updateProgress();
						} catch (InterruptedException e) {
							return;
						}
					}
				} finally {
					LOG.info("JBufferSink " + reduceID + " spill thread exit.");
				}
			}
		};
		spillThread.setDaemon(true);
		spillThread.start();
	}
	
	public TaskAttemptID reduceID() {
		return this.reduceID;
	}
	
	/**
	 * Close sink.
	 * No more connections can be made once closed. Method will
	 * lock the owning task object if snapshots are turned on.
	 */
	public synchronized void close() {
		if (this.acceptor == null) return; // Already done.
		try {
			this.acceptor.interrupt();
			this.server.close();
			this.acceptor = null;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		synchronized (connections) {
			boolean connectionsOpen = true;
			while (connectionsOpen) {
				connectionsOpen = false;
				for (List<Connection> clist : connections.values()) {
					for (Connection c : clist) {
						if (c.open) {
							LOG.debug("JBufferSink " + reduceID + 
									 ": close waiting for finish of " + c);
							connectionsOpen = true;
						}
					}
				}
				if (connectionsOpen) {
					try { connections.wait();
					} catch (InterruptedException e) { }
				}
			}
			this.spillThread.interrupt();
		}

		try {
			if (snapshots) {
				this.snapshotThread.close();
				LOG.info("JBufferSink " + reduceID + " snapshot thread closed.");
				synchronized (bufferRuns) {
					if (bufferRuns.size() != successful.size()) {
						LOG.error("JBufferSink: missing buffer runs!");
					}

					synchronized (task) {
						for (JBufferRun run : bufferRuns.values()) {
							LOG.info("JBufferSink " + reduceID + " apply run " + run.id);
							if (run.progress < 1.0f) {
								LOG.error("JBufferSink: final buffer run progress < 1.0f! " +
										" progress = " + run.progress);
							}
							run.spill(this.collector);
						}
					}
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		finally {
		}
	}
	
	private int spillid() {
		return this.spillCount++;
	}
	
	private void spill(Path data, Path index) {
		spillQueue.add(new SpillRun(data, index));
	}
	
	private JBufferCollector<K, V> buffer() {
		return this.collector;
	}
	
	public boolean complete() {
		return this.successful.size() == numConnections;
	}
	
	public void cancel(TaskAttemptID attempt) {
		synchronized (connections) {
			TaskID taskid = attempt.getTaskID();
			if (this.connections.containsKey(taskid)) {
				List<Connection> closed = new ArrayList<Connection>();
				for (Connection conn : this.connections.get(taskid)) {
					if (conn.id().equals(attempt)) {
						conn.close();
						closed.add(conn);
					}
				}
				this.connections.get(taskid).removeAll(closed);
			}
		}
	}
	
	private void done(Connection connection) {
		try {
			if (!connection.isSnapshot()) {
				LOG.debug("JBufferSink connection done. " + connection);
				synchronized (connections) {
					TaskID taskid = connection.id().getTaskID();
					if (this.connections.containsKey(taskid)) {
						if (connection.progress() == 1.0f) {
							this.successful.add(taskid);
						}
					}
					this.connections.notifyAll();

					/*
					if (safemode && collector instanceof JBuffer) {
						int minUncommittedSpillId = Integer.MAX_VALUE;
						for (List<Connection> clist : connections.values()) {
							for (Connection c : clist) {
								minUncommittedSpillId = Math.min(minUncommittedSpillId, c.minSpillId);
							}
						}
						((JBuffer)collector).minUnfinishedSpill(minUncommittedSpillId);
					}
					*/
				}
			}
		} finally {
			if (complete()) {
				updateProgress();
				LOG.debug("JBufferSink " + reduceID + " is complete.");
			}
		}
	}
	
	private void updateProgress() {
		float progress = (float) this.successful.size();
		Set<TaskID> taskids = new HashSet<TaskID>(connections.keySet());
		for (TaskID taskid : taskids) {
			if (!this.successful.contains(taskid)) {
				float max = 0f;
				for (Connection c : connections.get(taskid)) {
					max = Math.max(max, c.progress());
				}
				progress += max;
			}
		}
		collector.getProgress().set(progress / (float) numConnections);
		reporter.progress();
	}
	
	private JBufferRun getBufferRun(TaskID taskid) {
		synchronized (bufferRuns) {
			if (!this.bufferRuns.containsKey(taskid)) {
				this.bufferRuns.put(taskid, 
						            new JBufferRun(taskid));
			}
			return this.bufferRuns.get(taskid);
		}
	}
	
	/************************************** CONNECTION CLASS **************************************/
	
	private class Connection implements Runnable {
		private float progress;
		
		private TaskAttemptID id;
		
		private JBufferSink<K, V> sink;
		private DataInputStream input;
		
		private boolean open;
		private boolean busy = false;
		
		private int mergingSpills = 0;
		
		private boolean isSnapshot;
		
		public Connection(DataInputStream input, JBufferSink<K, V> sink, JobConf conf) throws IOException {
			this.input = input;
			this.sink = sink;
			this.progress = 0f;
			this.open = true;
			
			this.id = new TaskAttemptID();
			this.id.readFields(input);
			this.isSnapshot = input.readBoolean();
			
		}
		
		public boolean isSnapshot() {
			return this.isSnapshot;
		}
		
		public boolean equals(Object o) {
			if (o instanceof JBufferSink.Connection) {
				return ((Connection)o).id.equals(this.id);
			}
			return false;
		}
		
		public String toString() {
			StringBuilder sb = new StringBuilder();
			sb.append("Connection: receiving buffer " + id + ". ");
			sb.append("Applying to buffer " + reduceID + ". ");
			sb.append("Progress = " + progress + ". ");
			sb.append("Busy? = " + busy + ". ");
			return sb.toString();
		}
		
		public float progress() {
			return this.progress;
		}
		
		public TaskAttemptID id() {
			return this.id;
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
		spill(IFile.Reader<K, V> reader, long length, Class<K> keyClass, Class<V> valClass, CompressionCodec codec) 
		throws IOException {
			// Spill directory to disk
			int spillid = sink.spillid();
			Path filename      = outputFileManager.getOutputFileForWrite(id(), spillid, length);
			Path indexFilename = outputFileManager.getOutputIndexFileForWrite(id(), spillid, JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);

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

				JBufferCollector<K, V> buffer = sink.buffer();
				/* Don't want to hold up mapper. */
				sink.spill(filename, indexFilename);
			} catch (Throwable e) {
				LOG.error("JBufferSink: error " + e + " during spill. progress = " + progress);
				e.printStackTrace();
			}
		}
		
		private void service(long length) throws IOException {
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
			
			if (sink.snapshots()) {
				try {
					LOG.debug("JBufferSink: perform snaphot to buffer " + reduceID + " from buffer " + this.id + " progress = " + progress);
					JBufferRun run = sink.getBufferRun(this.id.getTaskID());
					run.snapshot(reader, length, progress);
					return;
				} catch (Throwable t) {
					LOG.warn("Snapshot interrupted by " + t);
					return; // don't care
				}
			} else {
				if (sink.task.isSnapshotting() || sink.task.isMerging()) {
					/* Drain socket while task is snapshotting. */
					LOG.debug("JBufferSink: call spill for buffer " + id + " due to merging or snapshotting.");
					spill(reader, length, keyClass, valClass, codec);
				} else { 
					boolean doSpill = true;
					JBufferCollector<K, V> buffer = sink.buffer();
					if (!safemode || progress == 1.0f) {
						LOG.debug("JBufferSink: get task lock for " + id + " dump.");
						synchronized (sink.task) {
							/* Try to add records to the buffer. 
							 * Note: this means we can't back out the records so
							 * if we're in safemode this needs to be the final answer.
							 */
							if (!safemode && sink.buffer().reserve(length)) {
								LOG.debug("JBufferSink: dumping buffer " + id + ".");
								int records = 0;
								try {
									while (reader.next(key, value)) {
										records++;
										this.sink.buffer().collect(key, value);
									}
								} catch (ChecksumException e) {
									e.printStackTrace();
									LOG.error("JBufferSink: ChecksumException during spill. progress = " + progress);
								} catch (Throwable t) {
									t.printStackTrace();
									LOG.error("JBufferSink: " + t + " during spill. progress = " + progress);
								} finally {
									LOG.debug("JBufferSink: dumped " + records + " records.");
									sink.buffer().unreserve(length);
									doSpill = false;
								}
							}
						}
					}
					
					if (doSpill) {
						LOG.debug("JBufferSink: had to spill " + id + ".");
						spill(reader, length, keyClass, valClass, codec);
					}
					else {
						updateProgress();
					}
				}
			}
		}
		
		public void run() {
			try {
				while (open) {
					long length = 0;
					boolean force = false;
					try {
						busy = false;
						length = this.input.readLong();
						this.progress = this.input.readFloat();
						busy = true;
					}
					catch (Throwable e) {
						return;
					}
					
					if (length == 0 && !force) {
						if (progress < 1f) continue;
						else  return;
					}
					else {
						long timestamp = System.currentTimeMillis();
						LOG.debug("JBufferSink connection " + id + " service length " + 
								  length + " progress = " + progress);
						service(length);
						LOG.debug("JBufferSink connection " + id + " service time = " + 
								  (System.currentTimeMillis() - timestamp));
						if (force) return;
					}
					
					if (progress == 1.0f) return;
				}
			} catch (Throwable e) {
				e.printStackTrace();
				return;
			}
			finally {
				busy = false;
				close(); // must be called before done!
				sink.done(this);
			}
		}
	}
}
