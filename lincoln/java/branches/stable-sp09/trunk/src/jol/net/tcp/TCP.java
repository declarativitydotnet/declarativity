package jol.net.tcp;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.Map;

import jol.core.Runtime;
import jol.net.Address;
import jol.net.Channel;
import jol.net.IP;
import jol.net.Message;
import jol.net.Network;
import jol.net.Server;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;

public class TCP extends Server {
	private static final TableName ReceiveMessage = new TableName("tcp", "receive");

	private Runtime context;

	private Network manager;

	private ServerSocket server;

	private ThreadGroup threads;

	private Map<Connection, Thread> channels;

	public TCP(Runtime context, Network manager, Integer port) throws IOException, UpdateException, JolRuntimeException {
		super("TCP Server");
		this.context = context;
		this.manager = manager;
		this.server = new ServerSocket(port);
		this.threads = new ThreadGroup("TCP");
		this.threads.setDaemon(true);
		this.channels = new HashMap<Connection, Thread>();

        // NB: we need to evaluate() here to avoid a race condition: the
        // TCP program must be registered before we accept any connections
		context.install("system", "jol/net/tcp/tcp.olg");
		context.evaluate();
	}

	@Override
	public void interrupt() {
		super.interrupt();

		// Interrupt each thread and wait for it to exit. We've already
		// closed the ServerSocket, so no new threads can be created
		for (Map.Entry<Connection, Thread> entry : this.channels.entrySet()) {
			entry.getValue().interrupt();
			entry.getKey().close();
			try {
				entry.getValue().join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		this.threads.destroy();
		this.channels.clear();
	}

	@Override
	public void run() {
		while (true) {
			Connection conn = null;
			try {
				if (this.server.isClosed())
					break;

				Socket socket = this.server.accept();
				conn = new Connection(socket);
				manager.connection().register(conn);

				Thread thread  = new Thread(this.threads, conn);
				thread.setDaemon(true);
				thread.start();
				channels.put(conn, thread);
			} catch (IOException e) {
				if (conn != null) conn.close();
				if (this.server.isClosed())
					break;
				throw new RuntimeException(e);
			} catch (UpdateException e) {
				if (conn != null) conn.close();
				e.printStackTrace();
			}
		}
	}

	@Override
	public Channel open(Address address) {
		try {
			Connection conn = new Connection(address);
			Thread thread = new Thread(this.threads, conn);
			thread.start();
			channels.put(conn, thread);
			return conn;
		} catch (Exception e) {
			return null;
		}
	}

	@Override
	public void close(Channel channel) {
		if (this.channels.containsKey(channel)) {
			((Connection) channel).close();
			this.channels.get(channel).interrupt();
			this.channels.remove(channel);
		}
	}

	@Override
    public void cleanup() {
	    try {
	        this.server.close();
	    } catch (IOException e) {
	        throw new RuntimeException(e);
	    }
    }

	private class Connection extends Channel implements Runnable {
		private Socket socket;
		private ObjectInputStream iss;
		private ObjectOutputStream oss;

		public Connection(Address address)
			throws UnknownHostException, IOException {
			super("tcp", address);
			IP ip = (IP) address;
			this.socket = new Socket(ip.address(), ip.port());
			this.oss    = new ObjectOutputStream(this.socket.getOutputStream());
			this.iss    = new ObjectInputStream(this.socket.getInputStream());
		}

		public Connection(Socket socket) throws IOException {
			super("tcp", new IP(socket.getInetAddress(), socket.getPort()));
			this.socket = socket;
			this.oss    = new ObjectOutputStream(this.socket.getOutputStream());
			this.iss    = new ObjectInputStream(this.socket.getInputStream());
		}

		@Override
		public boolean send(Message packet) {
			try {
				if (this.socket.isClosed())
					return false;

				this.oss.writeObject(packet);
			} catch (IOException e) {
				return false;
			}
			return true;
		}

		private void close() {
			try {
				this.socket.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		public void run() {
			while (true) {
				if (this.socket.isClosed())
					break;

				try {
					Message message = (Message) this.iss.readObject();
					IP address = new IP(this.socket.getInetAddress(), this.socket.getPort());
					Tuple tuple = new Tuple(address, message);
					context.schedule("tcp", ReceiveMessage, new BasicTupleSet(tuple), null);
				} catch (IOException e) {
					try {
						TCP.this.manager.connection().unregister(this);
					} catch (JolRuntimeException e1) {
						e1.printStackTrace();
					}
					return;
				} catch (ClassNotFoundException e) {
					e.printStackTrace();
					System.exit(0);
				} catch (Exception e) {
					e.printStackTrace();
					System.exit(0);
				}
			}
		}
	}
}
