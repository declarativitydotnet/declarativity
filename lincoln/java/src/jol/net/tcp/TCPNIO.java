package jol.net.tcp;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.Channels;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.Hashtable;

import jol.net.Address;
import jol.net.IP;
import jol.net.Channel;
import jol.net.Network;
import jol.net.Message;
import jol.net.Server;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;
import jol.core.Runtime;

public class TCPNIO extends Server {
	private static final TableName ReceiveMessage = new TableName("tcp", "receive");
	
	private Runtime context;
	
	private Network manager;
	
	private ServerSocketChannel server;
	
	private Selector selector;
	
	private Hashtable<String, Connection> connections;
		
	public TCPNIO(Runtime context, Network manager, Integer port) throws IOException, UpdateException {
		this.context = context;
		this.manager = manager;
		this.server = ServerSocketChannel.open();
		this.selector = Selector.open();
		this.connections = new Hashtable<String, Connection>();
		context.install("system", ClassLoader.getSystemClassLoader().getResource("jol/net/tcp/tcp.olg"));
		
		this.server.configureBlocking(false);
		this.server.socket().bind(new InetSocketAddress(port));
		this.server.register(this.selector, SelectionKey.OP_ACCEPT);
	}
	
	public void run() {
		int keys = 0;
		while (true) {
			try {
				while ((keys = this.selector.select(10)) == 0) ;
				
				for (SelectionKey key : this.selector.selectedKeys()) {
					if (key.isAcceptable() && key.channel() instanceof ServerSocketChannel) {
						ServerSocketChannel server = (ServerSocketChannel) key.channel();
						SocketChannel channel = server.accept();
						if (channel != null) {
							Connection connection = new Connection(channel);
							manager.connection().register(connection);
							register (connection);
						}
					}
					else if (key.isReadable() && key.channel() instanceof SocketChannel) {
						String connectionKey = ((SocketChannel)key.channel()).socket().toString();
						Connection connection = this.connections.get(connectionKey);
						connection.receive();
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			} catch (UpdateException e) {
				e.printStackTrace();
			}
		}
	}
	
	@Override
	public Channel open(Address address) {
		try {
			IP ip = (IP) address;
			SocketChannel channel = SocketChannel.open();
			channel.configureBlocking(false);
			channel.connect(new InetSocketAddress(ip.address(), ip.port()));
			while (!channel.finishConnect()) {
				Thread.yield();
			}
			Connection connection = new Connection(channel);
			register(connection);
			return connection;
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}
	
	@Override
	public void close(Channel channel) {
		if (channel instanceof Connection) {
			Connection connection = (Connection) channel;
			if (this.connections.containsKey(connection.toString())) {
				this.connections.remove(connection.toString());
			}
		}
	}
	
	private void register(Connection connection) throws ClosedChannelException {
		connections.put(connection.toString(), connection);
		connection.channel.register(this.selector, SelectionKey.OP_READ);
	}
	
	private class Connection extends Channel {
		private SocketChannel channel;

		public Connection(SocketChannel channel) throws IOException {
			super("tcp", new IP(channel.socket().getInetAddress(), channel.socket().getPort()));
			this.channel = channel;
			channel.configureBlocking(false);
		}
		
		@Override 
		public String toString() {
			return this.channel.socket().toString();
		}
		
		@Override
		public boolean send(Message packet) {
			try {
				if (!this.channel.isConnected()) {
					return false;
				}
				ByteArrayOutputStream bstream = new ByteArrayOutputStream();
				ObjectOutputStream ostream = new ObjectOutputStream(bstream);
				ostream.writeObject(packet);
				ByteBuffer buffer = ByteBuffer.allocate(bstream.size());
				buffer.put(bstream.toByteArray());
				buffer.flip();
				this.channel.write(buffer);
			} catch (IOException e) {
				return false;
			}
			return true;
		}
		
		@Override
		public void close() {
			try {
				this.channel.close();
			} catch (IOException e) { }
		}

		public void receive() {
			try {
				ByteBuffer buffer = ByteBuffer.allocate(256);
				int bytes = this.channel.read(buffer);
				System.err.println("READ " + bytes + " BYTES FROM CHANNEL");
				ObjectInputStream istream = new ObjectInputStream(new ByteArrayInputStream(buffer.array()));
				Message message = (Message) istream.readObject();
				IP address = new IP(this.channel.socket().getInetAddress(), this.channel.socket().getPort());
				Tuple tuple = new Tuple(address, message);
				context.schedule("tcp", ReceiveMessage, new TupleSet(ReceiveMessage, tuple), new TupleSet(ReceiveMessage));
			} catch (IOException e) {
				try {
					TCPNIO.this.manager.connection().unregister(this);
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
