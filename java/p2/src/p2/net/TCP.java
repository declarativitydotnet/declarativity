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

public class TCP extends ObjectTable {
	
	private static class Server implements Runnable {

		private ServerSocket server;
		
		private Hashtable<String, Thread> connections;
		
		public Server(Integer port) throws IOException {
			this.server = new ServerSocket(port);
			this.connections = new Hashtable<String, Thread>();
		}
		
		public void run() {
			while (true) {
				try {
					Socket socket = this.server.accept();
					Thread thread = new Thread(new Connection(socket));
					thread.start();
					connections.put(socket.getInetAddress().toString(), thread);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	private static class Connection implements Runnable {
		private Socket socket;
		private ObjectInputStream iss;

		public Connection(Socket socket) throws IOException {
			this.socket = socket;
			this.iss = new ObjectInputStream(socket.getInputStream());
		}

		public void run() {
			while(true) {
				try {
					Packet packet = (Packet) this.iss.readObject();
					p2.core.System.schedule(packet.program(), packet.insertions(), packet.deletions());
				} catch (IOException e) {
					e.printStackTrace();
					System.exit(0);
				} catch (ClassNotFoundException e) {
					e.printStackTrace();
				} catch (Exception e) {
					e.printStackTrace();
					System.exit(0);
				}
				
			}
		}
	}
	
	private static class TCPChannel extends Channel {
		
		private Socket socket;
		
		private ObjectOutputStream oss;
		
		public TCPChannel(String address) throws Exception {
			super(address);
			String  host = address.substring(0, address.indexOf(':'));
			Integer port = Integer.parseInt(address.substring(address.indexOf(':') + 1));
			this.socket = new Socket(host, port);
			this.oss = new ObjectOutputStream(this.socket.getOutputStream());
		}

		@Override
		public boolean send(Packet packet) {
			try {
				if (this.socket.isClosed()) {
					System.err.println("\tSOCKET CLOSED");
					return false;
				}
				
				this.oss.writeObject(packet);
				this.oss.flush();
			} catch (IOException e) {
				return false;
			}
			return true;
		}
		
		@Override
		public void close() {
			try {
				System.err.println("CLOSE SOCKET " + socket.getPort());
				socket.close();
			} catch (IOException e) { }
		}
		
	}
	
	public static final Key PRIMARY_KEY = new Key(0);
	
	public enum Field{DESTINATION, CHANNEL};
	public static final Class[] SCHEMA = {
		String.class,   // Destination Location
		Channel.class   // Channel
	};
	
	private Thread server;
	
	public TCP(Integer port) throws IOException {
		super(new TableName("network", "tcp"), PRIMARY_KEY, new TypeList(SCHEMA));
		server = new Thread(new Server(port));
		server.start();
	}
	
	
	@Override
	public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
		for (Tuple tuple : tuples) {
			String address = (String) tuple.value(Field.DESTINATION.ordinal());
			try {
				TCPChannel channel = new TCPChannel(address);
				tuple.value(Field.CHANNEL.ordinal(), channel);
			} catch (Exception e) {
				throw new UpdateException(e.toString());
			}
		}
		
		return super.insert(tuples, conflicts);
	}
	
	@Override
	public boolean delete(Tuple tuple) throws UpdateException {
		if (super.delete(tuple)) {
			TCPChannel channel = (TCPChannel) tuple.value(Field.CHANNEL.ordinal());
			if (channel != null) {
				channel.close();
			}
			return true;
		}
		return false;
	}
	

}
