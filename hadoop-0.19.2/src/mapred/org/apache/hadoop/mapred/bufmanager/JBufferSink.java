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
import java.util.Collection;
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
				System.err.println("NEW SNAPSHOT FILE " + data);
				FSDataOutputStream out  = localFs.create(data);
				FSDataOutputStream idx = localFs.create(index);
				if (out == null) throw new IOException("Unable to create snapshot " + data);
				write(reader, out, idx);
				this.length   = length;
				this.progress = progress;
				this.snapshot = data;
				this.index    = index;
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
			
			synchronized (bufferRuns) {
				bufferRuns.notifyAll();
				while (busy) {
					try {
						bufferRuns.wait();
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
		}
		
		public void snapshot() {
			synchronized (bufferRuns) {
				bufferRuns.notifyAll();
			}
		}
		
		public void run() {
			float progress = 0f;
			try {
				while (open) {
					List<JBufferSink.JBufferRun> runs = new ArrayList<JBufferSink.JBufferRun>();
					synchronized (bufferRuns) {
						busy = false;
						bufferRuns.notifyAll();

						int freshRuns = 0;
						do {
							if (!open) return;
							try { bufferRuns.wait();
							} catch (InterruptedException e) { return; }
							if (!open) return;

							freshRuns = 0;
							for (JBufferRun run : bufferRuns.values()) {
								if (run.fresh) freshRuns++;
							}
						} while (freshRuns < bufferRuns.size() / 3);

						runs.clear();
						progress = 0f;
						for (JBufferRun run : bufferRuns.values()) {
							if (run.valid()) {
								progress += run.progress;
								runs.add(run);
							}
						}
						progress = progress / (float) numConnections;
						
						busy = true;
					}

					try {
						if (!open) return;
						LOG.info("SnapshotThread: " + reduceID + " perform snapshot. progress " + progress);
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
				synchronized (bufferRuns) {
					open = false;
					busy = false;
					bufferRuns.notifyAll();
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
	
	private Map<TaskID, List<Connection>> connections;
	
	private Map<TaskID, JBufferRun> bufferRuns;
	
	private Set<TaskID> successful;
	
	private Task task;
	
	private SnapshotThread snapshotThread;
	
	private boolean snapshots;
	
	private ReduceOutputFile outputFileManager;
	
	public JBufferSink(JobConf conf, TaskAttemptID reduceID, JBufferCollector<K, V> collector, Task task, boolean snapshots) throws IOException {
		this.conf = conf;
		this.reduceID = reduceID;
		this.collector = collector;
		this.localFs = FileSystem.getLocal(conf);
		this.maxConnections = conf.getInt("mapred.reduce.parallel.copies", 20);
		this.outputFileManager = new ReduceOutputFile(reduceID);
		this.outputFileManager.setConf(conf);
		this.bufferRuns = new HashMap<TaskID, JBufferRun>();
		
		this.task = task;
	    this.numConnections = task.getNumberOfInputs();
		this.executor = Executors.newCachedThreadPool();
		this.connections = new HashMap<TaskID, List<Connection>>();
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
					while (server.isOpen()) {
						SocketChannel channel = server.accept();
						channel.configureBlocking(true);
						DataInputStream  input  = new DataInputStream(channel.socket().getInputStream());
						Connection       conn   = new Connection(input, JBufferSink.this, conf);
						
						synchronized (JBufferSink.this) {
							TaskID taskid = conn.id().getTaskID();
							DataOutputStream output = new DataOutputStream(channel.socket().getOutputStream());
							
							if (!connections.containsKey(taskid)) {
								connections.put(taskid, new ArrayList<Connection>());
							}
							
							if (successful.contains(taskid) || connections.size() > maxConnections) {
								output.writeBoolean(false); // Deny
								output.flush();
								conn.close();
							}
							else {
								output.writeBoolean(true); // Connection open
								output.flush();
								
								/* register connection. */
								connections.get(taskid).add(conn);
								executor.execute(conn);
							}
						}
					}
				} catch (IOException e) { }
			}
		};
		acceptor.setPriority(Thread.MAX_PRIORITY);
		acceptor.start();
	}
	
	public TaskAttemptID reduceID() {
		return this.reduceID;
	}
	
	private JBufferCollector<K, V> buffer() {
		return this.collector;
	}
	
	public boolean complete() throws IOException {
		return this.successful.size() == numConnections;
	}
	
	public void cancel(TaskAttemptID attempt) {
		synchronized (this) {
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
		synchronized (this) {
			TaskID taskid = connection.id().getTaskID();
			if (this.connections.containsKey(taskid)) {
				this.connections.get(taskid).remove(connection);
				if (connection.progress() == 1.0f) {
					this.successful.add(taskid);
					
					if (this.successful.size() == numConnections) {
						try {
							if (snapshots) {
								this.snapshotThread.close();
								synchronized (bufferRuns) {
									if (bufferRuns.size() != successful.size()) {
										LOG.error("JBufferSink: missing buffer runs!");
									}
									
									synchronized (task) {
										for (JBufferRun run : bufferRuns.values()) {
											if (run.progress < 1.0f) {
												LOG.error("JBufferSink: final buffer run progress < 1.0f! " +
														" progress = " + run.progress);
											}
											run.spill(this.collector);
										}
									}
								}
							}
							if (this.acceptor != null) acceptor.interrupt();
							this.server.close();
						} catch (IOException e) {
							e.printStackTrace();
						}
						finally {
							synchronized (task) {
								task.notifyAll();
							}
						}
					}
					for (Connection c : connections.get(taskid)) {
						c.close();
					}
					connections.remove(taskid);
				}
				this.notifyAll();
			}
		}
	}
	
	private void updateProgress() {
		synchronized (task) {
			float progress = (float) this.successful.size();
			for (List<Connection> clist : connections.values()) {
				float max = 0f;
				for (Connection c : clist) {
					max = Math.max(max, c.progress());
				}
				progress += max;
			}
			collector.getProgress().set(progress / (float) numConnections);
			task.notifyAll();
		}
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
		
		private void 
		spill(IFile.Reader<K, V> reader, long length, Class<K> keyClass, Class<V> valClass, CompressionCodec codec) 
		throws IOException {
			// Spill directory to disk
			Path filename      = outputFileManager.getOutputFileForWrite(id(), progress == 1f, length);
			Path indexFilename = outputFileManager.getOutputIndexFileForWrite(id(), progress == 1f, JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);

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
				/* Register the spill file with the buffer. */
				synchronized (sink.task) {
					buffer.spill(filename, indexFilename, false);
				}
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
						this.progress = this.input.readFloat();
					}
					catch (Throwable e) {
						return;
					}
					
					synchronized (this) {
						if (!open) {
							return;
						}
						busy = true; // busy
					}
					
					if (length == 0) {
						if (progress < 1f) continue;
						else {
							return;
						}
					}
					
					CompressionCodec codec = null;
					if (conf.getCompressMapOutput()) {
						Class<? extends CompressionCodec> codecClass =
							conf.getMapOutputCompressorClass(DefaultCodec.class);
						codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
					}
					IFile.Reader<K, V> reader = new IFile.Reader<K, V>(conf, input, length, codec);
					
					if (sink.snapshots()) {
						try {
							JBufferRun run = sink.getBufferRun(this.id.getTaskID());
							System.err.println("New snapshot for buffer " + reduceID + " progress " + progress);
							run.snapshot(reader, length, progress);
							System.err.println("Created snapshot for buffer " + reduceID + " progress " + progress);
						} catch (Throwable t) {
							System.err.println("Snapshot interrupted by " + t);
							return; // don't care
						}
					} else {
						if (sink.task.isSnapshotting()) {
							/* Drain socket while task is snapshotting. */
							spill(reader, length, keyClass, valClass, codec);
						} else { 
							boolean doSpill = true;
							JBufferCollector<K, V> buffer = sink.buffer();
							synchronized (sink.task) {
								if (sink.buffer().reserve(length)) {
									try {
										while (reader.next(key, value)) {
											this.sink.buffer().collect(key, value);
										}
									} catch (ChecksumException e) {
										LOG.error("JBufferSink: ChecksumException during spill. progress = " + progress);
									}
									finally {
										sink.buffer().unreserve(length);
										doSpill = false;
									}
								}
							}
							if (doSpill) spill(reader, length, keyClass, valClass, codec);
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
