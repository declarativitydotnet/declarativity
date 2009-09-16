package org.apache.hadoop.mapred.bufmanager;

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
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

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
import org.apache.hadoop.mapred.Task;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.util.ReflectionUtils;


public class JBufferSink<K extends Object, V extends Object> {
	private static final Log LOG = LogFactory.getLog(JBufferSink.class.getName());
	  
	public class Snapshot {
		private float progress;
		
		private Path data;
		
		private Path index;
		
		private long length;
		
		private boolean fresh;
		
		public Snapshot(Path data, Path index) {
			this.data = data;
			this.index = index;
			this.length = 0;
			this.fresh = false;
		}
		
		public String toString() {
			return "Snapshot: " + data;
		}
		
		public boolean valid() {
			return this.length > 0;
		}
		
		public float progress() {
			return this.progress;
		}
		
		public Path data() {
			return this.data;
		}
		
		public Path index() {
			return this.index;
		}
		
		public long length() {
			return this.length;
		}
		
		public synchronized void 
		write(IFile.Reader<K, V> reader, long length, Class<K> keyClass, Class<V> valClass, CompressionCodec codec, float progress) 
		throws IOException {
			FSDataOutputStream dataOut  = localFs.create(data);
			FSDataOutputStream indexOut = localFs.create(index);

			if (dataOut == null || indexOut == null) 
				throw new IOException("Unable to create snapshot " + data);

			DataInputBuffer key = new DataInputBuffer();
			DataInputBuffer value = new DataInputBuffer();
			IFile.Writer<K, V> writer = new IFile.Writer<K, V>(conf, dataOut,  keyClass, valClass, codec);
			/* Copy over the data until done. */
			while (reader.next(key, value)) {
				writer.append(key, value);
			}
			writer.close();
			dataOut.close();

			/* Write the index file. */
			indexOut.writeLong(0);
			indexOut.writeLong(writer.getRawLength());
			indexOut.writeLong(dataOut.getPos());

			/* Close everything. */
			indexOut.flush();
			indexOut.close();
			this.progress = progress;
			this.fresh = true;
			this.length = length;
		}
	}
	
	public class SnapshotThread extends Thread {
		private boolean busy = false;
		private boolean open = true;
		
		public void close() {
			if (!open) return;
			else open = false;
			
			System.err.println("SnapshotThread: close " + reduceID);
			synchronized (snapshotConnections) {
				snapshotConnections.notifyAll();
				while (busy) {
					System.err.println("\tSnapshotThread: close is busy " + reduceID);
					try {
						snapshotConnections.wait();
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
			System.err.println("SnapshotThread: done closing " + reduceID);
		}
		
		public void snapshot() {
			synchronized (snapshotConnections) {
				snapshotConnections.notifyAll();
			}
		}
		
		public void run() {
			List<JBufferSink.Snapshot> runs = new ArrayList<JBufferSink.Snapshot>();
			float progress = 0f;
			try {
				while (open) {
					synchronized (snapshotConnections) {
						busy = false;
						snapshotConnections.notifyAll();

						int freshConnections = 0;
						do {
							if (!open) return;
							try { snapshotConnections.wait();
							} catch (InterruptedException e) { return; }
							if (!open) return;

							freshConnections = 0;
							for (Connection conn : snapshotConnections) {
								if (conn.snapshot() != null && conn.snapshot().fresh) {
									freshConnections++;
								}
							}
						} while (freshConnections < snapshotConnections.size() / 3);

						progress = 0f;
						for (Connection conn : snapshotConnections) {
							Snapshot run = conn.snapshot();
							if (run != null && run.valid()) {
								run.fresh = false;
								progress += run.progress;
								runs.add(run);
							}
						}
						progress = progress / (float) numConnections;
						busy = true;
					}

					try {
						if (!open) return;
						System.err.println("SnapshotThread: " + reduceID + " perform snapshot. progress " + progress);
						boolean keepSnapshots = snapshotTask.snapshots(runs, progress);
						System.err.println("SnapshotThread: " + reduceID + " done with snapshot. progress " + progress);
						if (!keepSnapshots) {
							return;
						}
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
			finally {
				synchronized (snapshotConnections) {
					open = false;
					busy = false;
					snapshotConnections.notifyAll();
				}

			}
		}
	}
	
	private int maxConnections;
	
	private Executor executor;
	
	private FileSystem localFs;
	
	private Thread acceptor;
	
	private ServerSocketChannel server;
	
	private int numConnections;
	
	private JBufferCollector<K, V> collector;
	
	private JobConf conf;
	
	private TaskAttemptID reduceID;
	
	private Map<TaskAttemptID, List<Connection>> connections;
	
	private List<Connection> snapshotConnections;
	
	private Set<TaskAttemptID> runningTransfers;
	
	private Set<TaskID> successful;
	
	private Task snapshotTask;
	
	private SnapshotThread snapshotThread;
	
	public JBufferSink(JobConf conf, TaskAttemptID reduceID, JBufferCollector<K, V> collector, int numConnections) throws IOException {
		this.conf = conf;
		this.reduceID = reduceID;
		this.collector = collector;
		this.localFs = FileSystem.getLocal(conf);
		this.maxConnections = conf.getInt("mapred.reduce.parallel.copies", 20);
		
	    this.numConnections = numConnections;
		this.executor = Executors.newCachedThreadPool();
		this.connections = new HashMap<TaskAttemptID, List<Connection>>();
		this.successful = new HashSet<TaskID>();
		this.runningTransfers = new HashSet<TaskAttemptID>();
		this.snapshotConnections = new ArrayList<Connection>();
		this.snapshotTask = null;
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
	
	public void snapshot(Task task) {
		this.snapshotTask = task;
	}
	
	public void open() {
		this.acceptor = new Thread() {
			public void run() {
				try {
					while (server.isOpen()) {
						SocketChannel channel = server.accept();
						channel.configureBlocking(true);
						DataInputStream  input  = new DataInputStream(channel.socket().getInputStream());
						Connection       conn   = new Connection(input, JBufferSink.this, conf);
						
						System.err.println("JBufferSink: received " + conn);
						if (!conn.isSnapshot() && snapshotConnections.size() > 0) {
							System.err.println("JBufferSink: " + reduceID + " got "+ conn + ". close all snapshots");
							closeSnapshots();
						}
						
						synchronized (JBufferSink.this) {
							if (!conn.isSnapshot() && !connections.containsKey(conn.id())) {
								connections.put(conn.id(), new ArrayList<Connection>());
							}
							
							DataOutputStream output = new DataOutputStream(channel.socket().getOutputStream());
							if (conn.isSnapshot() &&  connections.size() > 0) {
								output.writeBoolean(false); // We've already accepted a non-snapshot connection
								output.flush();
								conn.close();
							}
							else if (!runningTransfers.contains(conn.id()) && runningTransfers.size() > maxConnections) {
								output.writeBoolean(false); // Connection not open
								output.flush();
								conn.close();
							}
							else {
								output.writeBoolean(true); // Connection open
								output.flush();
								
								/* register connection. */
								if (conn.isSnapshot()) {
									// snapshots
									synchronized (snapshotConnections) {
										snapshotConnections.add(conn);
									}
								}
								else {
									// regular
									connections.get(conn.id()).add(conn);
									runningTransfers.add(conn.id());
								}
								executor.execute(conn);
							}
						}
					}
				} catch (IOException e) { }
			}
		};
		acceptor.start();
	}
	
	public TaskAttemptID reduceID() {
		return this.reduceID;
	}
	
	private JBufferCollector<K, V> buffer() {
		return this.collector;
	}
	
	public void block() throws IOException {
		try {
			synchronized (this) {
				while (this.successful.size() < numConnections) {
					try { this.wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
			if (this.acceptor != null) acceptor.interrupt();
		} finally {
			this.server.close();
		}
	}
	
	public void cancel(TaskAttemptID taskid) {
		synchronized (this) {
			if (this.connections.containsKey(taskid)) {
				for (Connection conn : this.connections.get(taskid)) {
					conn.close();
				}
				this.connections.remove(taskid);
			}
		}
	}
	
	private void done(Connection connection) {
		synchronized (this) {
			if (this.connections.containsKey(connection.id())) {
				if (connection.progress() == 1.0f) {
					this.successful.add(connection.id().getTaskID());
					this.runningTransfers.remove(connection.id());
					
					if (this.successful.size() == numConnections) {
						try {
							collector.close();
						} catch (IOException e) {
							e.printStackTrace();
						}
					}
				}
				this.connections.get(connection.id()).remove(connection);
				this.notifyAll();
			}
		}
	}
	
	private void updateProgress() {
		synchronized (this) {
			float progress = (float) this.successful.size();
			for (List<Connection> clist : connections.values()) {
				float max = 0f;
				for (Connection c : clist) {
					max = Math.max(max, c.progress());
				}
				progress += max;
			}
			collector.getProgress().set(progress / (float) numConnections);
		}
	}
	
	private void updateSnapshot(Connection connection) {
		synchronized (this) {
			if (connections.size() > 0) {
				connection.close();
			}
			else if (snapshotTask != null) {
				this.snapshotThread.snapshot();
			}
		}
	}
	
	private void closeSnapshots() {
		synchronized (snapshotConnections) {
			for (Connection snapshot : snapshotConnections) {
				snapshot.close();
			}
			snapshotConnections.clear();
		}
		snapshotThread.close();
		snapshotTask = null;
	}
	
	/************************************** CONNECTION CLASS **************************************/
	
	private class Connection implements Runnable {
		private float progress;
		
		private Snapshot snapshot;
		
		private int snapshotRuns;
		
		private TaskAttemptID id;
		
		private JBufferSink<K, V> sink;
		private DataInputStream input;
		
		private ReduceOutputFile reduceOutputFile;
		
		private boolean busy;
		private boolean open;
		
		public Connection(DataInputStream input, JBufferSink<K, V> sink, JobConf conf) throws IOException {
			this.input = input;
			this.sink = sink;
			this.progress = 0f;
			this.busy = false;
			this.open = true;
			
			this.id = new TaskAttemptID();
			this.id.readFields(input);
			
			this.reduceOutputFile = new ReduceOutputFile(reduceID);
			this.reduceOutputFile.setConf(conf);
			
			boolean isSnapshot = input.readBoolean();
			this.snapshotRuns = isSnapshot ? 0 : -1;
		}
		
		private Snapshot createNewSnapshot() throws IOException {
			int run = this.snapshotRuns++;
			return new Snapshot(reduceOutputFile.getOutputSnapFileForWrite(id, run, 1096), 
						        reduceOutputFile.getOutputSnapIndexFileForWrite(id, run, 1096));
		}
		
		public String toString() {
			StringBuilder sb = new StringBuilder();
			sb.append("Connection: receiving buffer " + id + ". ");
			sb.append("Applying to buffer " + reduceID + ". ");
			sb.append("Snapshot? = " + isSnapshot());
			return sb.toString();
		}
		
		public float progress() {
			return this.progress;
		}
		
		public boolean isSnapshot() {
			return this.snapshotRuns >= 0;
		}
		
		public Snapshot snapshot() {
			return this.snapshot;
		}
		
		public TaskAttemptID id() {
			return this.id;
		}

		public void close() {
			synchronized (this) {
				open = false;
				while (busy) {
					try { this.wait();
					} catch (InterruptedException e) { }
				}
				
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
		
		public void run() {
			try {
				DataInputBuffer key = new DataInputBuffer();
				DataInputBuffer value = new DataInputBuffer();
				
				Class <K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
				Class <V> valClass = (Class<V>)conf.getMapOutputValueClass();
				
				while (open) {
					long length = 0;
					synchronized (this) {
						busy = false;     // not busy
						this.notifyAll(); // tell everyone
					}
					try {
						length = this.input.readLong();
					}
					catch (Throwable e) {
						return; // This is okay.
					}
					
					synchronized (this) {
						if (!open) return;
						busy = true; // busy
					}
					
					this.progress = this.input.readFloat();
					
					if (length == 0) {
						if (progress < 1f) continue;
						else return;
					}
					
					CompressionCodec codec = null;
					if (conf.getCompressMapOutput()) {
						Class<? extends CompressionCodec> codecClass =
							conf.getMapOutputCompressorClass(DefaultCodec.class);
						codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
					}
					IFile.Reader<K, V> reader = new IFile.Reader<K, V>(conf, input, length, codec);
					
					if (isSnapshot()) {
						LOG.debug("Connection " + id + " new snapshot. progress = " + progress);
						Snapshot run = createNewSnapshot();
						run.write(reader, length, keyClass, valClass, codec, progress);
						this.snapshot = run;
						sink.updateSnapshot(this);
					}
					else if (this.sink.buffer().reserve(length)) {
						try {
							while (reader.next(key, value)) {
								this.sink.buffer().collect(key, value);
							}
						} catch (ChecksumException e) {
							LOG.error("ReduceMapSink: ChecksumException during spill. progress = " + progress);
						}
						finally {
							this.sink.buffer().unreserve(length);
						}
					}
					else {
						// Spill directory to disk
						Path filename      = reduceOutputFile.getOutputFileForWrite(id(), progress == 1f, length);
						Path indexFilename = reduceOutputFile.getOutputIndexFileForWrite(id(), progress == 1f, JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);
						
						while (localFs.exists(filename)) {
							LOG.warn("File " + filename + " exists. Waiting....");
							Thread.sleep(100);
						}
						
						while (localFs.exists(indexFilename)) {
							LOG.warn("File " + indexFilename + " exists. Waiting....");
							Thread.sleep(100);
						}
						
						FSDataOutputStream out      = localFs.create(filename, false);
						FSDataOutputStream indexOut = localFs.create(indexFilename, false);
						
						if (out == null || indexOut == null) 
							throw new IOException("Unable to create spill file " + filename);
						
						IFile.Writer<K, V> writer = new IFile.Writer<K, V>(conf, out,  keyClass, valClass, codec);
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
							
							/* Register the spill file with the buffer. */
							this.sink.buffer().spill(filename, length, indexFilename, false);
							
						} catch (Throwable e) {
							LOG.error("JBufferSink: error " + e + " during spill. progress = " + progress);
							e.printStackTrace();
						}
						finally {
							if (localFs.exists(filename)) {
								LOG.warn(filename + " still exists!");
							}
							if (localFs.exists(indexFilename)) {
								LOG.warn(indexFilename + " still exists!");
							}
						}
					}
					
					sink.updateProgress();
					if (progress == 1.0f) return;
				}
			} catch (Throwable e) {
				e.printStackTrace();
				return;
			}
			finally {
				synchronized (this) {
					busy = false;
					sink.done(this);
					this.notifyAll();
					if (open) close();
				}
			}
		}
	}
}
