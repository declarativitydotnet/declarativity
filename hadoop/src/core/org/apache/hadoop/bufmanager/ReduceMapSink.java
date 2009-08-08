package org.apache.hadoop.bufmanager;

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
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.SerializationFactory;
import org.apache.hadoop.mapred.IFile;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.mapred.IFile.Reader;
import org.apache.hadoop.util.ReflectionUtils;


public class ReduceMapSink<K extends Object, V extends Object> {
	
	private Executor executor;
	
	private Thread acceptor;
	
	private ServerSocketChannel server;
	
	private int numMapTasks;
	
	private OutputCollector<K, V> collector;
	
	private JobConf conf;
	
	private Map<TaskAttemptID, Connection> connections;
	
	private Set<TaskID> successful;
	
	public ReduceMapSink(JobConf conf, OutputCollector<K, V> collector) throws IOException {
		this.conf = conf;
		this.collector = collector;
		
	    /** How many mappers? */
	    this.numMapTasks = conf.getNumMapTasks();
		this.executor = Executors.newFixedThreadPool(this.numMapTasks);
		this.connections = new HashMap<TaskAttemptID, Connection>();
		this.successful = new HashSet<TaskID>();
	    
		/** The server socket and selector registration */
		this.server = ServerSocketChannel.open();
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
						DataInputStream input = new DataInputStream(channel.socket().getInputStream());
						Connection conn = new Connection(input, ReduceMapSink.this, conf);
						synchronized (connections) {
							connections.put(conn.taskid(), conn);
						}
						executor.execute(conn);
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		};
		acceptor.start();
	}
	
	private synchronized void collect(K key, V value) throws IOException {
		this.collector.collect(key, value);
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
		synchronized (connections) {
			if (this.connections.containsKey(taskid)) {
				this.connections.get(taskid).cancel();
				this.connections.remove(taskid);
			}
		}
	}
	
	private void done(Connection connection) {
		synchronized (this) {
			this.successful.add(connection.taskid().getTaskID());
			if (this.successful.size() == numMapTasks) {
				this.notify();
			}
		}
	}

	
	
	/************************************** CONNECTION CLASS **************************************/
	
	private class Connection implements Runnable {
		private TaskAttemptID taskid;
		
		private IFile.Reader<K, V> reader;
		
	    private Deserializer<K> keyDeserializer;
	    private Deserializer<V> valDeserializer;
	    
		private ReduceMapSink<K, V> sink;
		
		private boolean cancel;
		
		public Connection(DataInputStream input, ReduceMapSink<K, V> sink, JobConf conf) throws IOException {
			this.sink = sink;
			this.cancel = false;
			
			/** How do unmarshall the key, value pairs. */
			Class keyClass = conf.getMapOutputKeyClass();
			Class valClass = conf.getMapOutputValueClass();
		    SerializationFactory serializationFactory = new SerializationFactory(conf);
		    this.keyDeserializer = serializationFactory.getDeserializer(keyClass);
		    this.valDeserializer = serializationFactory.getDeserializer(valClass);

			CompressionCodec codec = null;
			if (conf.getCompressMapOutput()) {
				Class<? extends CompressionCodec> codecClass =
					conf.getMapOutputCompressorClass(DefaultCodec.class);
				codec = (CompressionCodec) ReflectionUtils.newInstance(codecClass, conf);
			}
			
			this.taskid = new TaskAttemptID();
			this.taskid.readFields(input);
			long size = input.readLong();
			
			this.reader = new IFile.Reader<K, V>(conf, input, size, codec);
		}
		
		public TaskAttemptID taskid() {
			return this.taskid;
		}
		
		public void cancel() {
			try {
				this.cancel = true;
				this.reader.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		public void run() {
			try {
				DataInputBuffer key = new DataInputBuffer();
				DataInputBuffer value = new DataInputBuffer();
				while (this.reader.next(key, value)) {
					this.keyDeserializer.open(key);
					this.valDeserializer.open(value);
					K k = this.keyDeserializer.deserialize(null);
					V v = this.valDeserializer.deserialize(null);
					this.sink.collect(k, v);
				}
				if (!cancel) sink.done(this);
			} catch (Throwable e) {
				return;
			}
			finally {
				try {
					this.reader.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
}
