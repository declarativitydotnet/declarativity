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

	private Map<BufferID, List<BufferRequest>> requests;
	
	private Map<BufferID, Path> completedBuffers;
	
	private Map<JobID, Set<BufferID>> jobBuffers;

	private Server server;

	private ServerSocketChannel channel;
	
	private int controlPort;

	public BufferController(Configuration conf) throws IOException {
		this.conf             = conf;
		this.requests         = new HashMap<BufferID, List<BufferRequest>>();
		this.completedBuffers = new HashMap<BufferID, Path>();
		this.jobBuffers       = new HashMap<JobID, Set<BufferID>>();
		this.executor         = Executors.newCachedThreadPool();
		this.hostname         = InetAddress.getLocalHost().getCanonicalHostName();
	    this.localFs          = FileSystem.getLocal(conf);

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
	public BufferRequest getRequest(BufferID bufid) throws IOException {
		synchronized (this) {
			if (this.requests.containsKey(bufid) &&
					this.requests.get(bufid).size() > 0) {
				return this.requests.get(bufid).remove(0);
			}
			return null;
		}
	}

	@Override
	public void register(BufferID bufid, String output) throws IOException {
		synchronized (this) {
			System.err.println("BufferController: register final output " + output);
			this.completedBuffers.put(bufid, new Path(output));
			if (!this.jobBuffers.containsKey(bufid.taskid().getJobID())) {
				this.jobBuffers.put(bufid.taskid().getJobID(), new HashSet<BufferID>());
			}
			this.jobBuffers.get(bufid.taskid().getJobID()).add(bufid);
			
			if (this.requests.containsKey(bufid)) {
				for (BufferRequest request : this.requests.get(bufid)) {
					this.handleCompleteBuffers(request);
				}
				this.requests.remove(bufid);
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
			if (this.completedBuffers.containsKey(request.bufid())) {
				handleCompleteBuffers(request);
				return;
			}
			
			if (!this.requests.containsKey(request.bufid())) {
				this.requests.put(request.bufid(), new LinkedList<BufferRequest>());
			}
			this.requests.get(request.bufid()).add(request);
		}

	}
	
	private void handleCompleteBuffers(BufferRequest request) throws IOException {
		if (this.completedBuffers.containsKey(request.bufid())) {
			Path file = this.completedBuffers.get(request.bufid());
			FSDataInputStream fsin = localFs.open(file);
			request.open(request.bufid(), fsin);
			this.executor.execute(request);
		}
	}

	@Override
	public long getProtocolVersion(String protocol, long clientVersion)
	throws IOException {
		// TODO Auto-generated method stub
		return 0;
	}



}
