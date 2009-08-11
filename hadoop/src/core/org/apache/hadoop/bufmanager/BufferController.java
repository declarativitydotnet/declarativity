package org.apache.hadoop.bufmanager;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.ipc.RPC.Server;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.ReflectionUtils;

public class BufferController extends Thread implements BufferUmbilicalProtocol {

    protected final FileSystem localFs;
    
	private String hostname;
	
	private Configuration conf;

	private Executor executor;

	private Map<TaskAttemptID, TreeSet<BufferRequest>> requests;
	
	private Set<TaskAttemptID> committed;
	
	private Server server;

	private ServerSocketChannel channel;
	
	private int controlPort;

	public BufferController(Configuration conf) throws IOException {
		this.conf      = conf;
		this.requests  = new HashMap<TaskAttemptID, TreeSet<BufferRequest>>();
		this.committed = new HashSet<TaskAttemptID>();
		this.executor  = Executors.newCachedThreadPool();
		this.hostname  = InetAddress.getLocalHost().getCanonicalHostName();
	    this.localFs   = FileSystem.getLocal(conf);

	}

	public static InetSocketAddress getControlAddress(Configuration conf) {
		try {
			int port = conf.getInt("mapred.buffer.manager.data.port", 9021);
			String address = InetAddress.getLocalHost().getCanonicalHostName();
			address += ":" + port;
			return NetUtils.createSocketAddr(address);
		} catch (Throwable t) {
			return NetUtils.createSocketAddr("localhost:9021");
		}
	}

	public static InetSocketAddress getServerAddress(Configuration conf) {
		try {
			String address = InetAddress.getLocalHost().getCanonicalHostName();
			int port = conf.getInt("mapred.buffer.manager.control.port", 9020);
			address += ":" + port;
			return NetUtils.createSocketAddr(address);
		} catch (Throwable t) {
			return NetUtils.createSocketAddr("localhost:9020");
		}
	}

	private void initialize() throws IOException {
		int maxMaps = conf.getInt(
				"mapred.tasktracker.map.tasks.maximum", 2);
		int maxReduces = conf.getInt(
				"mapred.tasktracker.reduce.tasks.maximum", 2);

		InetSocketAddress serverAddress = getServerAddress(conf);
		this.server = RPC.getServer(this, serverAddress.getHostName(), serverAddress.getPort(), 
				maxMaps + maxReduces, false, conf);
		this.server.start();

		/** The server socket and selector registration */
		InetSocketAddress controlAddress = getControlAddress(conf);
		this.controlPort = controlAddress.getPort();
		this.channel = ServerSocketChannel.open();
		this.channel.socket().bind(controlAddress);
	}
	
	public void free(JobID jobid) {
	}
	
	@Override
	public BufferRequest getRequest(TaskAttemptID taskid) throws IOException {
		synchronized (this) {
			if (this.requests.containsKey(taskid) &&
					this.requests.get(taskid).size() > 0) {
				BufferRequest r = this.requests.get(taskid).first();
				this.requests.get(taskid).remove(r);
				return r;
			}
			return null;
		}
	}

	@Override
	public void commit(TaskAttemptID taskid) throws IOException {
		synchronized (this) {
			System.err.println("BufferController: register final output " + taskid);
			
			this.committed.add(taskid);
			
			if (this.requests.containsKey(taskid)) {
				handleCompleteBuffers(taskid);
			}
		}
	}

	@Override
	public void request(BufferRequest request) throws IOException {
		synchronized (this) {
			if (request.source().equals(hostname)) {
				register(request); // Request is local!
			}
			else {
				Socket socket = null;
				try {
					InetSocketAddress controlSource = NetUtils.createSocketAddr(request.source() + ":" + this.controlPort);
					socket = new Socket();
					socket.connect(controlSource);
					DataOutputStream out = new DataOutputStream(socket.getOutputStream());
					request.write(out);
					out.flush();

				} catch (IOException e) {
					if (socket != null) socket.close();
					throw e;
				}
			}
		}
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
				SocketChannel channel = this.channel.accept();
				BufferRequest request = new BufferRequest();
				DataInputStream in = new DataInputStream(channel.socket().getInputStream());
				request.readFields(in);
				
				register(request);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	
	private void register(BufferRequest request) throws IOException {
		synchronized(this) {
			if (!this.requests.containsKey(request.taskid())) {
				this.requests.put(request.taskid(), new TreeSet<BufferRequest>());
			}
			this.requests.get(request.taskid()).add(request);
			
			if (this.committed.contains(request.taskid())) {
				handleCompleteBuffers(request.taskid());
			}
		}

	}
	
	private void handleCompleteBuffers(TaskAttemptID taskid) throws IOException {
		if (this.requests.containsKey(taskid)) {
			for (BufferRequest request : this.requests.get(taskid)) {
				request.open(this.conf, this.localFs);
				request.flushFile();
			}
			this.requests.remove(taskid);
		}
	}

	@Override
	public long getProtocolVersion(String protocol, long clientVersion)
	throws IOException {
		// TODO Auto-generated method stub
		return 0;
	}



}
