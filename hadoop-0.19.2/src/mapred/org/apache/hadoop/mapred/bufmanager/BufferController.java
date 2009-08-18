package org.apache.hadoop.mapred.bufmanager;

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
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.io.compress.CodecPool;
import org.apache.hadoop.io.compress.CompressionCodec;
import org.apache.hadoop.io.compress.DefaultCodec;
import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.ipc.RPC.Server;
import org.apache.hadoop.mapred.IFile;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.JobID;
import org.apache.hadoop.mapred.MapOutputFile;
import org.apache.hadoop.mapred.TaskAttemptID;
import org.apache.hadoop.mapred.TaskID;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.ReflectionUtils;

public class BufferController extends Thread implements BufferUmbilicalProtocol {
	
	private class PipelineHandler implements Runnable {
		private TaskAttemptID taskid;
		
		private TreeSet<BufferRequest> requests;
		
		private MapOutputFile mapOutputFile;
		
		private boolean open;
		
		private int curSpillId;
		
		private int maxSpillId;
		
		public PipelineHandler(TaskAttemptID taskid, TreeSet<BufferRequest> requests) {
			this.taskid = taskid;
			this.requests = requests;
			this.open     = true;
			this.curSpillId = 0;
			this.maxSpillId = 0;
			
			this.mapOutputFile = new MapOutputFile(taskid.getJobID());
			this.mapOutputFile.setConf(conf);
		}
		
		public void close() {
			synchronized (this) {
				this.open = false;
				this.notifyAll();
			}
		}
		
		public boolean maxSpillId(int id) throws IOException {
			synchronized (this) {
				if (this.maxSpillId < id && this.curSpillId == this.maxSpillId) {
					this.maxSpillId++;
					this.notifyAll();
					return true;
				}
				return false;
			}
		}

		public void run() {
			while (true) {
				synchronized (this) {
					while (open && curSpillId == maxSpillId) {
						try { this.wait(); } catch (InterruptedException e) { }
					}
				}
				try {
					while (this.curSpillId < this.maxSpillId) {
						Path outputFile = mapOutputFile.getSpillFile(this.taskid, this.curSpillId);
						Path indexFile  = mapOutputFile.getSpillIndexFile(this.taskid, this.curSpillId);
						FSDataInputStream indexIn = localFs.open(indexFile);
						FSDataInputStream dataIn = localFs.open(outputFile);

						for (BufferRequest request : requests) {
							request.flush(indexIn, dataIn, false);
						}
						indexIn.close();
						dataIn.close();
						this.curSpillId++;
					}
					
					if (! open && this.curSpillId == this.maxSpillId) {
						Path finalIndexFile  = mapOutputFile.getOutputIndexFile(this.taskid);
						Path finalOutputFile = mapOutputFile.getOutputFile(this.taskid);
						FSDataInputStream indexIn = localFs.open(finalIndexFile);
						FSDataInputStream dataIn  = localFs.open(finalOutputFile);

						for (BufferRequest request : requests) {
							request.flush(indexIn, dataIn, true);
							request.close();
						}
						indexIn.close();
						dataIn.close();
						return;
					}
				} catch (IOException e) {
					e.printStackTrace();
					close();
				}
			}
		}

	}

    protected final FileSystem localFs;
    
	private String hostname;
	
	private Configuration conf;

	private Executor executor;

	private Map<TaskAttemptID, TreeSet<BufferRequest>> requests;
	
	private Map<TaskAttemptID, PipelineHandler> pipelineHandlers;
	
	private Set<TaskAttemptID> committed;
	
	private Server server;

	private ServerSocketChannel channel;
	
	private int controlPort;

	public BufferController(Configuration conf) throws IOException {
		this.conf      = conf;
		this.requests  = new HashMap<TaskAttemptID, TreeSet<BufferRequest>>();
		this.pipelineHandlers = new HashMap<TaskAttemptID, PipelineHandler>();
		
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
	public boolean pipe(TaskAttemptID taskid, int id, int numReduces) throws IOException {
		synchronized (this) {
			if (this.pipelineHandlers.containsKey(taskid)) {
				return this.pipelineHandlers.get(taskid).maxSpillId(id);
			}
			else if (this.requests.containsKey(taskid) && this.requests.get(taskid).size() == numReduces) {
				TreeSet<BufferRequest> requests = this.requests.get(taskid);
				
				for (BufferRequest request : requests) {
					request.open(conf, localFs);
				}
				
				PipelineHandler handler = new PipelineHandler(taskid, requests);
				this.pipelineHandlers.put(taskid, handler);
				this.executor.execute(handler);
				return handler.maxSpillId(id);
			}
			
			return false;
		}
	}

	@Override
	public void commit(TaskAttemptID taskid) throws IOException {
		synchronized (this) {
			this.committed.add(taskid);
			
			if (this.pipelineHandlers.containsKey(taskid)) {
				this.pipelineHandlers.get(taskid).close();
				this.pipelineHandlers.remove(taskid);
			}
			else if (this.requests.containsKey(taskid)) {
				handleCompleteBuffers(taskid);
			}
		}
	}
	
	@Override
	public BufferRequest getRequest(TaskAttemptID taskid) throws IOException {
		synchronized (this) {
			if (this.requests.containsKey(taskid)) {
				for (BufferRequest request : this.requests.get(taskid)) {
					if (request.delivered == false) {
						request.delivered = true;
						return request;
					}
				}
			}
			return null;
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
				this.executor.execute(request);
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
