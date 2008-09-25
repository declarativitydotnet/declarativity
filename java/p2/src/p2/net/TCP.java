package p2.net;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Hashtable;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

public class TCP extends Server {
	
	private Manager manager;
		
	private ServerSocket server;
	
	private Hashtable<String, Thread> channels;
		
	public TCP(Manager manager, Integer port) throws IOException {
		this.manager = manager;
		this.server = new ServerSocket(port);
		this.channels = new Hashtable<String, Thread>();
	}
	
	public void run() {
		while (true) {
			try {
				Socket     socket  = this.server.accept();
				Connection channel = new Connection(socket);
				Thread     thread  = new Thread(channel);
				thread.start();
				channels.put(channel.address(), thread);
			} catch (IOException e) {
				e.printStackTrace();
				return;
			}
		}
	}
	
	@Override
	public Channel open(String address) {
		try {
			Connection channel = new Connection(address);
			Thread thread = new Thread(channel);
			thread.start();
			channels.put(channel.address(), thread);
			return channel;
		} catch (Exception e) {
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
	
	private static class Connection extends Channel implements Runnable {
		private Socket socket;
		private ObjectInputStream iss;
		private ObjectOutputStream oss;

		public Connection(String address) 
			throws UnknownHostException, IOException {
			super("tcp", address);
			String  host = address.substring(0, address.indexOf(':'));
			Integer port = Integer.parseInt(address.substring(address.indexOf(':')+1));
			this.socket = new Socket(host, port);
			this.oss    = new ObjectOutputStream(this.socket.getOutputStream());
			this.iss    = new ObjectInputStream(this.socket.getInputStream());
		}
		
		public Connection(Socket socket) throws IOException {
			super("tcp", socket.getInetAddress().toString());
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
					Application packet = (Application) this.iss.readObject();
					p2.core.System.schedule(packet.program(), packet.insertions(), packet.deletions());
				} catch (IOException e) {
					close();
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
