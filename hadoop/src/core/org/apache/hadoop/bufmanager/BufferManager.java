package org.apache.hadoop.bufmanager;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.ipc.RPC.Server;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.ReflectionUtils;

public class BufferManager extends Thread implements BufferUmbilicalProtocol {
	interface BufferControl<K extends Object, V extends Object> extends Buffer<K, V> {
		/**
		 * Add record to the buffer.
		 * @param record Record to add.
		 * @throws IOException If I don't like you
		 */
		public void add(Record<K, V> record) throws IOException;

		/**
		 * @return The compression codec that is used to compress/decompress
		 * the data in this buffer.
		 */
		public CompressionCodec codec();

		/**
		 * @return Amount of main memory currently being used 
		 * by this buffer.
		 */
		public long memory();

		/**
		 * Flush all main memory contents to disk.
		 * @throws IOException
		 */
		public void flush() throws IOException;

		/**
		 * Free all memory (RAM/disk) associate with this buffer.
		 */
		public void free();
		
		/**
		 * Close the buffer. No records can be inserted
		 * when buffer is closed. This also marks the end
		 * of the buffer for all running stream iterators.
		 */
		public void close();
	}

	private Configuration conf;

	private Executor executor;

	private Map<BufferID, BufferControl> buffers;

	private Map<BufferID, Map<TaskAttemptID, Iterator<Record>>> iterators;

	private Map<BufferID, Map<TaskAttemptID, BufferReceiver>> fetches;
	
	private Map<BufferID, BufferRequest> outstanding;

	private Server bufServer;

	private ServerSocketChannel dataServer;

	private int dataPort;

	public BufferManager(Configuration conf) {
		this.conf        = conf;
		this.buffers     = new HashMap<BufferID, BufferControl>();
		this.fetches     = new HashMap<BufferID, Map<TaskAttemptID, BufferReceiver>>();
		this.iterators   = new HashMap<BufferID, Map<TaskAttemptID, Iterator<Record>>>();
		this.outstanding = new HashMap<BufferID, BufferRequest>();
		this.executor    = Executors.newCachedThreadPool();
		this.dataPort    = 0;
	}

	public static InetSocketAddress getDataAddress(Configuration conf) {
		try {
			int port = conf.getInt("mapred.buffer.manager.data.port", 9011);
			String address = InetAddress.getLocalHost().getCanonicalHostName();
			address += ":" + port;
			return NetUtils.createSocketAddr(address);
		} catch (Throwable t) {
			return NetUtils.createSocketAddr("localhost:9011");
		}
	}

	public static InetSocketAddress getControlAddress(Configuration conf) {
		try {
			String address = InetAddress.getLocalHost().getCanonicalHostName();
			int port = conf.getInt("mapred.buffer.manager.control.port", 9010);
			address += ":" + port;
			return NetUtils.createSocketAddr(address);
		} catch (Throwable t) {
			return NetUtils.createSocketAddr("localhost:9010");
		}
	}

	private void initialize() throws IOException {
		int maxMaps = conf.getInt(
				"mapred.tasktracker.map.tasks.maximum", 2);
		int maxReduces = conf.getInt(
				"mapred.tasktracker.reduce.tasks.maximum", 2);

		InetSocketAddress bindAddress = getControlAddress(conf);
		this.bufServer = RPC.getServer(this, bindAddress.getHostName(), bindAddress.getPort(), 
				maxMaps + maxReduces, false, conf);
		this.bufServer.start();

		/** The server socket and selector registration */
		InetSocketAddress dataAddress = getDataAddress(conf);
		this.dataPort = dataAddress.getPort();
		this.dataServer = ServerSocketChannel.open();
		this.dataServer.socket().bind(dataAddress);
	}

	@Override
	public void run() {
		try {
			initialize();
		} catch (IOException e) {
			e.printStackTrace();
		}

		while (true) {
			try {
				SocketChannel channel = this.dataServer.accept();
				BufferRequest request = new BufferRequest();
				DataInputStream in = new DataInputStream(channel.socket().getInputStream());
				request.readFields(in);

				synchronized(buffers) {
					if (this.buffers.containsKey(request.bufid())) {
						System.err.println("EXECUTE REQUEST " + request.bufid());
						BufferControl buffer = this.buffers.get(request.bufid());
						request.initialize(buffer, channel.socket());
						this.executor.execute(request);
					}
					else {
						System.err.println("OUTSTANDING REQUEST " + request.bufid());
						request.initialize(null, channel.socket());
						this.outstanding.put(request.bufid(), request);
					}
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}

	/**
	 * Called solely by the BufferReceiver class to indicate
	 * its completion.
	 * @param receiver
	 */
	void done(BufferReceiver receiver) {
		synchronized (fetches) {
			BufferID bufid = receiver.buffer().bufid();
			
			this.fetches.get(bufid).remove(receiver.tid());
			if (this.fetches.get(bufid).size() == 0) {
				this.fetches.remove(bufid);
				fetches.notifyAll();
			}
		}
	}

	@Override
	public long getProtocolVersion(String protocol, long clientVersion)
	throws IOException {
		// TODO Auto-generated method stub
		return 0;
	}


	@Override
	public void add(BufferID bufid, Record record) throws IOException {
		synchronized (buffers) {
			if (this.buffers.containsKey(bufid)) {
				BufferControl buffer = this.buffers.get(bufid);
				record.unmarshall(buffer.conf());
				buffer.add(record);
				if (buffer.memory() > 10000) {
					System.err.println("FLUSH BUFFER " + bufid + " SIZE " + buffer.memory());
					buffer.flush();
				}
				return;
			}
		}
		throw new IOException("Unknown buffer " + bufid);
	}

	@Override
	public BufferID create(TaskAttemptID tid, int partition, String jobFile, Buffer.BufferType type) 
	throws IOException {
		BufferID bufid = new BufferID(tid, partition);
		synchronized (buffers) {
			if (this.buffers.containsKey(bufid)) {
				throw new IOException("Buffer already exists! " + bufid);
			}

			if (type != Buffer.BufferType.GROUPED) {
				this.buffers.put(bufid, new JBuffer(new JobConf(jobFile), bufid, type));
			}
			else {
				this.buffers.put(bufid, new JBufferGroup(new JobConf(jobFile), bufid));
			}
			
			if (this.outstanding.containsKey(bufid)) {
				System.err.println("EXECUTE OUTSTANDING REQUEST " + bufid);
				BufferRequest request = this.outstanding.remove(bufid);
				request.initialize(this.buffers.get(bufid), request.socket());
				this.executor.execute(request);
			}
		}
		
		return bufid;
	}

	@Override
	public void fetch(BufferID destination, TaskAttemptID tid, String source) 
	throws IOException {
		BufferControl destBuffer = null;
		synchronized (buffers) {
			if (!this.buffers.containsKey(destination)) {
				throw new IOException("Unknown destination buffer! " + destination);
			}
			destBuffer = this.buffers.get(destination);

			Socket socket = null;

			try {
				BufferRequest request = new BufferRequest(new BufferID(tid, destination.partition()));
				InetSocketAddress src = NetUtils.createSocketAddr(source + ":" + this.dataPort);
				socket = new Socket();
				socket.connect(src);
				DataOutputStream out = new DataOutputStream(socket.getOutputStream());
				request.write(out);
				out.flush();

				BufferReceiver fetch = new BufferReceiver(this, destBuffer, tid, socket, destBuffer.codec());
				synchronized (fetches) {
					if (!this.fetches.containsKey(destination)) {
						this.fetches.put(destination, new HashMap<TaskAttemptID, BufferReceiver>());
					}
					this.fetches.get(destination).put(tid, fetch);
				}
				this.executor.execute(fetch);
			} catch (IOException e) {
				if (socket != null) socket.close();
				throw e;
			}
		}
	}

	@Override
	public void cancelFetch(BufferID destination, TaskAttemptID tid) throws IOException {
		synchronized (fetches) {
			if (this.fetches.containsKey(destination) &&
					this.fetches.get(destination).containsKey(tid)) {
				this.fetches.get(destination).get(tid).cancel();
				done(this.fetches.get(destination).get(tid));
			}
		}
	}

	@Override
	public Record getNextRecord(BufferID bufid, TaskAttemptID tid) throws IOException {
		synchronized (fetches) {
			while (this.fetches.containsKey(bufid) &&
					this.fetches.get(bufid).size() > 0) {
				try {
					fetches.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}

		synchronized (buffers) {
			if (!this.buffers.containsKey(bufid)) {
				throw new IOException("Uknown buffer identifier " + bufid);
			}

			if (!this.iterators.containsKey(bufid)) {
				this.iterators.put(bufid, new HashMap<TaskAttemptID, Iterator<Record>>());
			}

			if (!this.iterators.get(bufid).containsKey(tid)) {
				this.iterators.get(bufid).put(tid, this.buffers.get(bufid).records());
			}
		}

		Iterator<Record> iter = this.iterators.get(bufid).get(tid);
		if (iter.hasNext()) {
			Record record = iter.next();
			record.marshall(this.buffers.get(bufid).conf());
			return record;
		}
		else {
			this.iterators.get(bufid).remove(tid);
			return null;
		}
	}


	public void free(BufferID bufid) throws IOException {
		synchronized (buffers) {
			
			/* Cancel outstanding fetches */
			synchronized (fetches) {
				if (this.fetches.containsKey(bufid)) {
					for (BufferReceiver receiver : this.fetches.get(bufid).values()) {
						receiver.cancel();
					}
					this.fetches.remove(bufid);
				}
			}
			
			if (this.buffers.containsKey(bufid)) {
				this.buffers.get(bufid).free();
				this.buffers.remove(bufid);
				return;
			}
		}
		throw new IOException("BufferManager::free -- Unknown buffer! " + bufid);
	}
	
	public void close(BufferID bufid) throws IOException {
		synchronized (fetches) {
			while (this.fetches.containsKey(bufid) &&
					this.fetches.get(bufid).size() > 0) {
				try {
					fetches.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
		
		synchronized (buffers) {
			if (this.buffers.containsKey(bufid)) {
				this.buffers.get(bufid).close();
				return;
			}
		}
		throw new IOException("BufferManager::free -- Unknown buffer! " + bufid);
		
	}

}
