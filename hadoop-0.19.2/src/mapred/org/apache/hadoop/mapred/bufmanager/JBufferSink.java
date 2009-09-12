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
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.util.ReflectionUtils;


public class JBufferSink<K extends Object, V extends Object> {
	
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
	
	private Set<TaskAttemptID> runningTransfers;
	
	private Set<TaskID> successful;
	
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
					while (server.isOpen()) {
						SocketChannel channel = server.accept();
						channel.configureBlocking(true);
						DataInputStream  input  = new DataInputStream(channel.socket().getInputStream());
						Connection       conn   = new Connection(input, JBufferSink.this, conf);
						synchronized (this) {
							if (!connections.containsKey(conn.mapTaskID())) {
								connections.put(conn.mapTaskID(), new ArrayList<Connection>());
							}
							
							DataOutputStream output = new DataOutputStream(channel.socket().getOutputStream());
							if (!runningTransfers.contains(conn.mapTaskID()) && runningTransfers.size() > maxConnections) {
								output.writeBoolean(false); // Connection not open
								conn.close();
							}
							else {
								output.writeBoolean(true); // Connection open
								output.flush();
								connections.get(conn.mapTaskID()).add(conn);
								executor.execute(conn);
								runningTransfers.add(conn.mapTaskID());
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
			if (this.connections.containsKey(connection.mapTaskID())) {
				if (connection.progress() == 1.0f) {
					this.successful.add(connection.mapTaskID().getTaskID());
					this.runningTransfers.remove(connection.mapTaskID());
					
					if (this.successful.size() == numConnections) {
						try {
							collector.close();
						} catch (IOException e) {
							e.printStackTrace();
						}
					}
				}
				this.connections.get(connection.mapTaskID()).remove(connection);
				this.notifyAll();
			}
		}
	}
	
	private void updateProgress() {
		synchronized (this) {
			float progress = 0f;
			for (List<Connection> clist : connections.values()) {
				float max = 0f;
				for (Connection c : clist) {
					max = Math.max(max, c.progress());
				}
				progress += max;
			}
			collector.getProgress().set(progress / (float) numConnections);
			System.err.println("JBufferSink: progress = " + collector.getProgress().get());
		}
	}
	
	/************************************** CONNECTION CLASS **************************************/
	
	private class Connection implements Runnable {
		private float progress;
		
		private TaskAttemptID mapTaskID;
		
		private JBufferSink<K, V> sink;
		private DataInputStream input;
		
		private ReduceOutputFile reduceOutputFile;
		
		public Connection(DataInputStream input, JBufferSink<K, V> sink, JobConf conf) throws IOException {
			this.input = input;
			this.sink = sink;
			this.progress = 0f;
			
			this.mapTaskID = new TaskAttemptID();
			this.mapTaskID.readFields(input);
			
			this.reduceOutputFile = new ReduceOutputFile(reduceID);
			this.reduceOutputFile.setConf(conf);
		}
		
		public float progress() {
			return this.progress;
		}
		
		public TaskAttemptID mapTaskID() {
			return this.mapTaskID;
		}
		
		public void close() {
			try {
				this.input.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		public void run() {
			try {
				DataInputBuffer key = new DataInputBuffer();
				DataInputBuffer value = new DataInputBuffer();
				
				CompressionCodec codec = null;
				if (conf.getCompressMapOutput()) {
					Class<? extends CompressionCodec> codecClass =
						conf.getMapOutputCompressorClass(DefaultCodec.class);
					codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
				}
				Class <K> keyClass = (Class<K>)conf.getMapOutputKeyClass();
				Class <V> valClass = (Class<V>)conf.getMapOutputValueClass();
				
				while (true) {
					long length = 0;
					try {
						length = this.input.readLong();
					}
					catch (EOFException e) {
						return; // This is okay.
					}
					
					this.progress = this.input.readFloat();
					
					if (length == 0) return;
					
					IFile.Reader<K, V> reader = new IFile.Reader<K, V>(conf, input, length, codec);
					if (this.sink.buffer().reserve(length)) {
						try {
							while (reader.next(key, value)) {
								this.sink.buffer().collect(key, value);
							}
						} catch (ChecksumException e) {
							System.err.println("ReduceMapSink: ChecksumException during spill. progress = " + progress);
						}
						finally {
							this.sink.buffer().unreserve(length);
						}
					}
					else {
						// Spill directory to disk
						Path filename      = reduceOutputFile.getOutputFileForWrite(this.mapTaskID, progress == 1f, length);
						Path indexFilename = reduceOutputFile.getOutputIndexFileForWrite(this.mapTaskID, progress == 1f, JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);
						
						while (localFs.exists(filename)) {
							System.err.println("File " + filename + " exists. Waiting....");
							Thread.sleep(100);
						}
						
						while (localFs.exists(indexFilename)) {
							System.err.println("File " + indexFilename + " exists. Waiting....");
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
							this.sink.buffer().spill(filename, length, indexFilename);
							
						} catch (Throwable e) {
							System.err.println("ReduceMapSink: error during spill. progress = " + progress);
							e.printStackTrace();
						}
						finally {
							if (localFs.exists(filename)) {
								System.err.println("Warn: " + filename + " still exists!");
							}
							if (localFs.exists(indexFilename)) {
								System.err.println("Warn: " + indexFilename + " still exists!");
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
				sink.done(this);
				close();
			}
		}
	}
}
