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
import java.nio.channels.spi.SelectorProvider;
import java.util.Hashtable;
import java.util.Iterator;

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
		this.selector = SelectorProvider.provider().openSelector();
		this.connections = new Hashtable<String, Connection>();
		context.install("system", ClassLoader.getSystemClassLoader().getResource("jol/net/tcp/tcp.olg"));
		
		this.server.configureBlocking(false);
		this.server.socket().bind(new InetSocketAddress(port));
		this.server.register(this.selector, SelectionKey.OP_ACCEPT);
	}
	
	public void run() {
		while (true) {
			try {
				for (int keys = 0; keys == 0; keys = this.selector.select(10)) ;
				
		        // Iterate over the set of keys for which events are available
		        Iterator selectedKeys = this.selector.selectedKeys().iterator();
		        while (selectedKeys.hasNext()) {
		          SelectionKey key = (SelectionKey) selectedKeys.next();
		          selectedKeys.remove();

					if (!key.isValid()) {
						continue;
					}
					if (key.isAcceptable()) {
						ServerSocketChannel ssc = (ServerSocketChannel) key.channel();
						SocketChannel channel = ssc.accept();
						if (channel != null) {
							Connection connection = new Connection(channel);
							manager.connection().register(connection);
							register (connection);
						}
					}
					else if (key.isReadable()) {
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
		private ByteBuffer buffer;
		private SocketChannel channel;

		public Connection(SocketChannel channel) throws IOException {
			super("tcp", new IP(channel.socket().getInetAddress(), channel.socket().getPort()));
			this.buffer = ByteBuffer.allocate(10000);
			this.channel = channel;
			this.channel.configureBlocking(false);
		}
		
		@Override 
		public String toString() {
			return this.channel.socket().toString();
		}
		
		@Override
		public boolean send(Message packet) {
			synchronized (buffer) {
				try {
					if (!this.channel.isConnected()) {
						return false;
					}
					this.buffer.clear();
					ByteArrayOutputStream bstream = new ByteArrayOutputStream();
					ObjectOutputStream ostream = new ObjectOutputStream(bstream);
					ostream.writeObject(packet);
					buffer.putInt(bstream.size());
					buffer.put(bstream.toByteArray());
					buffer.flip();
					writeFully(this.buffer, this.channel);
				} catch (IOException e) {
					return false;
				}
				return true;
			}
		}
		
		@Override
		public void close() {
			try {
				this.channel.close();
			} catch (IOException e) { }
		}

		public void receive() {
			synchronized (buffer) {
				try {
					this.buffer.clear();
					this.buffer.limit(Integer.SIZE / Byte.SIZE);
					readFully(this.buffer, this.channel);

					int length = this.buffer.getInt(0);
					this.buffer.clear();
					this.buffer.limit(length);
					readFully(this.buffer, this.channel);

					ObjectInputStream istream = new ObjectInputStream(new ByteArrayInputStream(this.buffer.array()));
					Message message = (Message) istream.readObject();
					IP address = new IP(this.channel.socket().getInetAddress(), this.channel.socket().getPort());
					Tuple tuple = new Tuple(address, message);
					context.schedule("tcp", ReceiveMessage, new TupleSet(ReceiveMessage, tuple), new TupleSet(ReceiveMessage));
				} catch (IOException e) {
					e.printStackTrace();
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
		
	    private void writeFully(ByteBuffer buf, SocketChannel socket) throws IOException {
	        int len = buf.limit() - buf.position();
	        while (len > 0) {
	            len -= socket.write(buf);
	        }
	    }
	 
	    private void readFully(ByteBuffer buf, SocketChannel socket) throws IOException {
	        int len = buf.limit() - buf.position();
	        while (len > 0) {
	            len -= socket.read(buf);
	        }
	    }
	}

}
