package org.apache.hadoop.mapred.bufmanager;

import java.io.DataInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.channels.spi.SelectorProvider;
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

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.ChecksumException;
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
import org.apache.hadoop.mapred.IFile;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapOutputFile;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.IFile.Reader;
import org.apache.hadoop.util.ReflectionUtils;


public class ReduceMapSink<K extends Object, V extends Object> {
	
	private Executor executor;
	
	private FileSystem localFs;
	
	private Thread acceptor;
	
	private ServerSocketChannel server;
	
	private int numMapTasks;
	
	private ReduceOutputCollector<K, V> collector;
	
	private JobConf conf;
	
	private Map<TaskAttemptID, List<Connection>> connections;
	
	private Set<TaskID> successful;
	
	public ReduceMapSink(JobConf conf, ReduceOutputCollector<K, V> collector) throws IOException {
		this.conf = conf;
		this.collector = collector;
		this.localFs = FileSystem.getLocal(conf);
		
	    /** How many mappers? */
	    this.numMapTasks = conf.getNumMapTasks();
		this.executor = Executors.newFixedThreadPool(this.numMapTasks);
		this.connections = new HashMap<TaskAttemptID, List<Connection>>();
		this.successful = new HashSet<TaskID>();
	    
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
						DataInputStream input = new DataInputStream(channel.socket().getInputStream());
						Connection conn = new Connection(input, ReduceMapSink.this, conf);
						synchronized (this) {
							if (!connections.containsKey(conn.taskid())) {
								connections.put(conn.taskid(), new ArrayList<Connection>());
							}
							connections.get(conn.taskid()).add(conn);
						}
						executor.execute(conn);
					}
				} catch (IOException e) { }
			}
		};
		acceptor.start();
	}
	
	private ReduceOutputCollector<K, V> buffer() {
		return this.collector;
	}
	
	public void block() throws IOException {
		try {
			synchronized (this) {
				while (this.successful.size() < numMapTasks) {
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
			if (this.connections.containsKey(connection.taskid())) {
				this.successful.add(connection.taskid().getTaskID());
				this.notifyAll();
			}
		}
	}

	
	
	/************************************** CONNECTION CLASS **************************************/
	
	private class Connection implements Runnable {
		private TaskAttemptID taskid;
		
		private ReduceMapSink<K, V> sink;
		private DataInputStream input;
		
		private MapOutputFile mapOutputFile;
		
		private boolean open;
		
		public Connection(DataInputStream input, ReduceMapSink<K, V> sink, JobConf conf) throws IOException {
			this.input = input;
			this.sink = sink;
			this.open = true;
			
			this.taskid = new TaskAttemptID();
			this.taskid.readFields(input);
			
			this.mapOutputFile = new MapOutputFile(this.taskid.getJobID());
			this.mapOutputFile.setConf(conf);
		}
		
		public TaskAttemptID taskid() {
			return this.taskid;
		}
		
		public void close() {
			try {
				this.open = false;
				this.input.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		private void spill(long size) throws IOException {
			Path filename = mapOutputFile.getOutputFileForWrite(this.taskid, size);
			FSDataOutputStream out = localFs.create(filename);
			if (out == null ) throw new IOException("Unable to create spill file " + filename);
			
			byte[] buffer = new byte[Math.min((int)size, 1500)];
			int total = (int) size;
			while (total > 0) {
				int bytes = this.input.read(buffer, 0, Math.min(total, buffer.length));
				total -= bytes;
				out.write(buffer);
			}
			
			// create spill index
			Path indexFilename = mapOutputFile.getOutputIndexFileForWrite(this.taskid, JBuffer.MAP_OUTPUT_INDEX_RECORD_LENGTH);
			FSDataOutputStream indexOut = localFs.create(indexFilename);
			
			indexOut.writeLong(0);
			indexOut.writeLong(size);
			indexOut.writeLong(size);
			
			this.sink.buffer().spill(filename, size, indexFilename);
		}

		public void run() {
			boolean done = false;
			try {
				DataInputBuffer key = new DataInputBuffer();
				DataInputBuffer value = new DataInputBuffer();
				
				CompressionCodec codec = null;
				if (conf.getCompressMapOutput()) {
					Class<? extends CompressionCodec> codecClass =
						conf.getMapOutputCompressorClass(DefaultCodec.class);
					codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
				}
				
				while (true) {
					long length = this.input.readLong();
					done = this.input.readBoolean();
					
					System.err.println("Connection " + taskid + " sending " + length + " bytes.");
					
					if (this.sink.buffer().reserve(length)) {
						IFile.Reader reader = new IFile.Reader<K, V>(conf, input, length, codec);
						while (open && reader.next(key, value)) {
							this.sink.buffer().collect(key, value);
						}
						this.sink.buffer().unreserve(length);
					}
					else {
						// Spill directory to disk
						System.err.println("Spill connection " + taskid + " directly to disk");
						spill(length);
					}
					
					if (done) return;
				}
			} catch (ChecksumException e) {
				System.err.println("CONNECTION CLOSED FROM TASK " + taskid);
				// Ignore due to TCP close
			} catch (Throwable e) {
				e.printStackTrace();
				return;
			}
			finally {
				if (done) System.err.println("CONNECTION DONE: TASK " + taskid);
				if (done) sink.done(this);
				close();
			}
		}
	}
}
