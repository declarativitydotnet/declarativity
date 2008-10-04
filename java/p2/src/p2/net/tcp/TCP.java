package p2.net.tcp;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Hashtable;

import p2.net.Address;
import p2.net.IP;
import p2.net.Channel;
import p2.net.Network;
import p2.net.Message;
import p2.net.Server;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.UpdateException;
import p2.types.table.TableName;

public class TCP extends Server {
	private static final TableName ReceiveMessage = new TableName("tcp", "receive");
	
	private Network manager;
	
	private ServerSocket server;
	
	private Hashtable<Address, Thread> channels;
		
	public TCP(Network manager, Integer port) throws IOException, UpdateException {
		this.manager = manager;
		this.server = new ServerSocket(port);
		this.channels = new Hashtable<Address, Thread>();
		p2.core.System.install("TCP",
				ClassLoader.getSystemClassLoader().getResource("p2/net/tcp/tcp.olg").getPath());
	}
	
	public void run() {
		while (true) {
			Connection channel = null;
			try {
				Socket socket = this.server.accept();
				
				channel = new Connection(socket);
				manager.connection().register(channel);
				
				Thread thread  = new Thread(channel);
				thread.start();
				channels.put(channel.address(), thread);
			} catch (IOException e) {
				if (channel != null) channel.close();
				e.printStackTrace();
			} catch (UpdateException e) {
				if (channel != null) channel.close();
				e.printStackTrace();
			}
		}
	}
	
	@Override
	public Channel open(Address address) {
		try {
			Connection channel = new Connection(address);
			Thread thread = new Thread(channel);
			thread.start();
			channels.put(channel.address(), thread);
			return channel;
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}
	
	@Override
	public void close(Channel channel) {
		if (channels.containsKey(channel.address())) {
			channels.get(channel.address()).interrupt();
			channels.remove(channel.address());
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
			this.iss    = new ObjectInputStream(socket.getInputStream());
		}
		
		@Override
		public boolean send(Message packet) {
			try {
				if (this.socket.isClosed()) {
					return false;
				}
				this.oss.writeObject(packet);
			} catch (IOException e) {
				return false;
			}
			return true;
		}
		
		@Override
		public void close() {
			try {
				socket.close();
			} catch (IOException e) { }
		}

		public void run() {
			while(true) {
				try {
					Message message = (Message) this.iss.readObject();
					IP address = new IP(this.socket.getInetAddress(), this.socket.getPort());
					Tuple tuple = new Tuple(address, message);
					p2.core.System.schedule("tcp", ReceiveMessage, new TupleSet(ReceiveMessage, tuple), new TupleSet(ReceiveMessage));
				} catch (IOException e) {
					try {
						TCP.this.manager.connection().unregister(this);
					} catch (UpdateException e1) {
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
