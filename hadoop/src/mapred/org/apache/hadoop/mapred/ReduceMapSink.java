package org.apache.hadoop.mapred;

import java.io.EOFException;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.channels.spi.SelectorProvider;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.RawComparator;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.io.serializer.Deserializer;
import org.apache.hadoop.io.serializer.SerializationFactory;
import org.apache.hadoop.util.ReflectionUtils;


public class ReduceMapSink {
	
	private Executor executor;
	
	private ServerSocketChannel server;
	
	private int numMapTasks;
	
	private ReduceRecordMap records;
	
	private JobConf conf;
	
	private List<Connection<Object, Object>> connections;
	
	public ReduceMapSink(JobConf conf) throws IOException {
		this.conf = conf;
		
	    /** How many mappers? */
	    this.numMapTasks = conf.getNumMapTasks();
		this.executor = Executors.newFixedThreadPool(this.numMapTasks);
		this.connections = new LinkedList<Connection<Object, Object>>();

	    
	    /** How do we compare the Key portion of each record. */
	    RawComparator comparator = conf.getOutputKeyComparator();
	    this.records = new ReduceRecordMap(comparator);
	    
		/** The server socket and selector registration */
		this.server = ServerSocketChannel.open();
		this.server.socket().bind(new InetSocketAddress(0));
	}
	
	public ReduceRecordMap records() {
		return this.records;
	}
	
	synchronized void record(Object key, Object value) {
		this.records.add(key, value);
	}
	
	public String getAddress() {
		return this.server.socket().getInetAddress().getHostAddress() + 
		       ":" + this.server.socket().getLocalPort();
	}
	
	public boolean fetchOutputs() throws IOException {
		int finishedMaps = 0;
		try {
			while (connections.size() < numMapTasks) {
				SocketChannel channel = this.server.accept();
				Connection conn = new Connection(channel, this, conf);
				this.connections.add(conn);
				this.executor.execute(conn);
			}
			
			while (connections.size() > 0) {
				synchronized (this) {
					try { this.wait();
					} catch(Exception e) {}
				}
				
				Iterator<Connection<Object, Object>> iter = this.connections.iterator();
				while (iter.hasNext()) {
					Connection c = iter.next();
					if (!c.channel.isOpen()) {
						iter.remove();
					}
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		finally {
			this.server.close();
		}
		return true;
	}

	
	
	/************************************** CONNECTION CLASS **************************************/
	
	private class Connection<K, V> implements Runnable {
		private SocketChannel channel;
		
		private IFile.Reader<K, V> reader;
		
	    private Deserializer<Object> keyDeserializer;
	    private Deserializer<Object> valDeserializer;
	    
		private ReduceMapSink sink;
		
		public Connection(SocketChannel channel, ReduceMapSink sink, JobConf conf) throws IOException {
			this.channel = channel;
			this.sink = sink;

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

			this.reader = new IFile.Reader<K, V>(conf, channel.socket().getInputStream(), Integer.MAX_VALUE, codec);
		}

		public void run() {
			try {
				while (true) {
					DataInputBuffer key = new DataInputBuffer();
					DataInputBuffer value = new DataInputBuffer();

					if (this.reader.next(key, value)) {
						this.keyDeserializer.open(key);
						this.valDeserializer.open(value);
						Object k = this.keyDeserializer.deserialize(null);
						Object v = this.valDeserializer.deserialize(null);
						this.sink.record(k, v);
					}
				}
			} catch (Throwable e) {
				return;
			}
			finally {
				try {
					this.channel.close();
					synchronized(sink) {
						sink.notify();
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
}
