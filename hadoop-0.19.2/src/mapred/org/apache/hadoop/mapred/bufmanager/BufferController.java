package org.apache.hadoop.mapred.bufmanager;

import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ConnectException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.Collections;
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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
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
import org.apache.hadoop.mapred.TaskTracker;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.util.ReflectionUtils;

public class BufferController extends Thread implements BufferUmbilicalProtocol {
	private static final Log LOG = LogFactory.getLog(BufferController.class.getName());

	private class RequestTransfer extends Thread {
		private Map<InetSocketAddress, Set<BufferRequest>> transfers;
		
		public RequestTransfer() {
			this.transfers = new HashMap<InetSocketAddress, Set<BufferRequest>>();
		}
		
		public void transfer(BufferRequest request) {
			synchronized(transfers) {
				InetSocketAddress source = 
					NetUtils.createSocketAddr(request.source() + ":" + controlPort);
				if (!transfers.containsKey(source)) {
					transfers.put(source, new HashSet<BufferRequest>());
				}
				transfers.get(source).add(request);
				transfers.notify();
			}
		}
		
		public void run() {
			Set<InetSocketAddress> locations = new HashSet<InetSocketAddress>();
			Set<BufferRequest>     handle    = new HashSet<BufferRequest>();
			while (!isInterrupted()) {
				synchronized (transfers) {
					while (transfers.size() == 0) {
						try { transfers.wait();
						} catch (InterruptedException e) { }
					}
					locations.clear();
					locations.addAll(transfers.keySet());
				}
				
				for (InetSocketAddress location : locations) {
					synchronized(transfers) {
						handle.clear();
						handle.addAll(transfers.get(location));
					}
					Socket socket = null;
					DataOutputStream out = null;
					try {
						socket = new Socket();
						socket.connect(location);
						out = new DataOutputStream(new BufferedOutputStream(socket.getOutputStream()));
						for (BufferRequest request : handle) {
							request.write(out);
						}
						out.flush();
						
						synchronized (transfers) {
							transfers.get(location).removeAll(handle);
							if (transfers.get(location).size() == 0) {
								transfers.remove(location);
							}
						}
					} catch (IOException e) {
						LOG.warn("BufferController: connection issue " + e);
					} finally {
						try {
							if (out != null) {
								out.close();
							}
							
							if (socket != null) {
								socket.close();
							}
						} catch (Throwable t) {
							LOG.error(t);
						}
					}
				}

			}
		}

	};
	
	private class RequestHandler extends Thread {
		public void run() {
			List<TaskAttemptID> handleCommitted = new ArrayList<TaskAttemptID>();
			while (true) {
				handleCommitted.clear();
				handleCommitted.addAll(committed);

				for (TaskAttemptID taskid : handleCommitted) {
					List<BufferRequest> handleRequests = new ArrayList<BufferRequest>();
					synchronized (requests) {
						if (requests.containsKey(taskid)) {
							handleRequests.addAll(requests.get(taskid));
						}
					}
					
					if (handleRequests.size() > 0) {
						handleRequests = handle(taskid, handleRequests);
						
						synchronized (requests) {
							requests.get(taskid).removeAll(handleRequests);
							if (requests.get(taskid).size() == 0) {
								requests.remove(taskid);
							}
						}
					}
				}

				synchronized (requests) {
					try {
						if (requests.size() > 0) {
							requests.wait(1000);
						}
						else {
							requests.wait();
						}
					} catch (InterruptedException e) { }
				}
			}
		}
		
		private List<BufferRequest> handle(TaskAttemptID taskid, List<BufferRequest> handle) {
			List<BufferRequest> handled = new ArrayList<BufferRequest>();
			try {
				JobConf job = tracker.getJobConf(taskid);
				BufferRequestResponse response = new BufferRequestResponse();
				for (BufferRequest request : handle) {
					response.reset();
					request.open(job, response, false);
					if (response.open) { 
						executor.execute(request);
						handled.add(request);
					}
					else if (response.terminated) {
						LOG.info("BufferController: request terminated by receiver. " + request);
						handled.add(request); // throw away
					}
					else {
						request.connectionAttempts++;
						if (request.connectionAttempts > 10) {
							LOG.info("BufferController: retry request " + request + " too many times = " + request.connectionAttempts);
						}
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
			return handled;
		}
	}
	
    private TaskTracker tracker;
    
    private RequestHandler requestHandler;
    
    private RequestTransfer requestTransfer;
    
	private String hostname;
	
	private Executor executor;

	private Map<TaskAttemptID, TreeSet<BufferRequest>> requests = new HashMap<TaskAttemptID, TreeSet<BufferRequest>>();
	
	private Set<TaskAttemptID> committed = Collections.synchronizedSet(new HashSet<TaskAttemptID>());
	
	private Server server;

	private ServerSocketChannel channel;
	
	private int controlPort;
	

	public BufferController(TaskTracker tracker) throws IOException {
		this.tracker   = tracker;
		this.requestHandler = new RequestHandler();
		this.requestTransfer    = new RequestTransfer();
		this.executor  = Executors.newCachedThreadPool();
		this.hostname  = InetAddress.getLocalHost().getCanonicalHostName();
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
		Configuration conf = tracker.conf();
		int maxMaps = conf.getInt("mapred.tasktracker.map.tasks.maximum", 2);
		int maxReduces = conf.getInt("mapred.tasktracker.reduce.tasks.maximum", 1);

		InetSocketAddress serverAddress = getServerAddress(conf);
		this.server = RPC.getServer(this, serverAddress.getHostName(), serverAddress.getPort(), 
				maxMaps + maxReduces, false, conf);
		this.server.start();

		this.requestHandler.start();
		this.requestTransfer.start();
		
		/** The server socket and selector registration */
		InetSocketAddress controlAddress = getControlAddress(conf);
		this.controlPort = controlAddress.getPort();
		this.channel = ServerSocketChannel.open();
		this.channel.socket().bind(controlAddress);
	}
	
	public void free(JobID jobid) {
		synchronized (requests) {
			Set<TaskAttemptID> clean = new HashSet<TaskAttemptID>();
			for (TaskAttemptID taskid : this.requests.keySet()) {
				if (taskid.getJobID().equals(jobid)) {
					clean.add(taskid);
					for (BufferRequest request : this.requests.get(taskid)) {
						try { request.close();
						} catch (IOException e) { }
					}
				}
			}
			for (TaskAttemptID taskid : clean) {
				this.requests.remove(taskid);
			}
		}
	}
	
	@Override
	public void commit(TaskAttemptID taskid) throws IOException {
		synchronized (requests) {
			this.committed.add(taskid);
			this.requests.notifyAll();
		}
	}
	
	@Override
	public BufferRequest getRequest(TaskAttemptID taskid) throws IOException {
		synchronized (requests) {
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
	
	public void remove(BufferRequest request) throws IOException {
		synchronized (requests) {
			this.requests.get(request.taskid()).remove(request);
		}
	}

	@Override
	public void request(BufferRequest request) throws IOException {
		synchronized (requests) {
			if (request.source().equals(hostname)) {
				register(request); // Request is local.
			}
			else {
				requestTransfer.transfer(request); // request is remote.
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
			SocketChannel channel = null;
			try {
				channel = this.channel.accept();
				BufferRequest request = new BufferRequest();
				DataInputStream in = new DataInputStream(channel.socket().getInputStream());
				request.readFields(in);
				
				register(request);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			finally {
				try {
					if (channel != null) channel.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
	}
	
	private void register(BufferRequest request) throws IOException {
		synchronized(requests) {
			if (!this.requests.containsKey(request.taskid())) {
				this.requests.put(request.taskid(), new TreeSet<BufferRequest>());
			}
			this.requests.get(request.taskid()).add(request);
			this.requests.notifyAll();
		}

	}
	
	@Override
	public long getProtocolVersion(String protocol, long clientVersion)
	throws IOException {
		// TODO Auto-generated method stub
		return 0;
	}



}
