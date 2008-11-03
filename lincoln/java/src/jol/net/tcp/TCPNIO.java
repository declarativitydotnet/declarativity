package jol.net.tcp;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.channels.spi.SelectorProvider;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import jol.core.Runtime;
import jol.net.Address;
import jol.net.Channel;
import jol.net.IP;
import jol.net.Message;
import jol.net.Network;
import jol.net.Server;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;

public class TCPNIO extends Server {
	/** The name of the receive message predicate (tcp::receive). */
	private static final TableName ReceiveMessage = new TableName("tcp", "receive");
	
	/** The maximum number of times we will read/write a socket channel with no progress. */
	private static final int MAX_SOCKET_ATTEMPTS = 10;
	
	private Runtime context;
	
	private Network manager;
	
	private Selector selector;
	
	private ExecutorService executor;
	
	private List<Connection> newConnections;
		
	public TCPNIO(Runtime context, Network manager, Integer port) throws IOException, UpdateException {
		super("TCPNIO Server");
		this.context = context;
		this.manager = manager;
		this.selector = SelectorProvider.provider().openSelector();
		this.executor = Executors.newFixedThreadPool(java.lang.Runtime.getRuntime().availableProcessors());
		this.newConnections = new ArrayList<Connection>();
		context.install("system", "jol/net/tcp/tcp.olg");
		
		ServerSocketChannel server = ServerSocketChannel.open();
		server.configureBlocking(false);
		server.socket().bind(new InetSocketAddress(port));
		server.register(this.selector, SelectionKey.OP_ACCEPT);
	}
	
	@Override
	public void interrupt() {
		super.interrupt();
		this.executor.shutdown();
	}
	
	@Override
	public void run() {
		while (true) {
			try {
				this.selector.select();

				/* Add any pending new connections to the select set */
				synchronized (this.newConnections) {
					for (Connection newConn : this.newConnections) {
						SelectionKey key = newConn.channel.register(this.selector, SelectionKey.OP_READ);
						key.attach(newConn);
					}
					this.newConnections.clear();
				}
				
		        // Iterate over the set of keys for which events are available
				Iterator<SelectionKey> iter = this.selector.selectedKeys().iterator();
				while (iter.hasNext()) {
					SelectionKey key = iter.next();
					iter.remove();

					if (!key.isValid())
					    continue;
					
					if (key.isAcceptable()) {
						ServerSocketChannel ssc = (ServerSocketChannel) key.channel();
						SocketChannel channel = ssc.accept();
						if (channel != null) {
							Connection connection = new Connection(channel);
							manager.connection().register(connection);
							register(connection);
						}
					}
					else if (key.isReadable()) {
						final Connection connection = (Connection) key.attachment();
						executor.execute(new Runnable() {
							public void run() {
								connection.receive();
							}
						});
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
			return null;
		}
	}
	
	@Override
	public void close(Channel channel) {
		if (channel instanceof Connection) {
			Connection connection = (Connection) channel;
			connection.close();
		}
	}
	
	private void register(Connection connection) {
		synchronized (this.newConnections) {
			this.newConnections.add(connection);
			this.selector.wakeup();
		}
	}
	
	private class Connection extends Channel {
		private ByteBuffer buffer;
		private SocketChannel channel;

		public Connection(SocketChannel channel) throws IOException {
			super("tcp", new IP(channel.socket().getInetAddress(), channel.socket().getPort()));
			this.buffer = ByteBuffer.allocate(8192);
			this.channel = channel;
			this.channel.configureBlocking(false);
		}
		
		@Override 
		public String toString() {
			return this.channel.socket().toString();
		}
		
		@Override
		public boolean send(Message packet) {
			try {
				synchronized (buffer) {
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
					write(this.buffer, this.channel);
				}
			} catch (IOException e) {
				return false;
			}
			return true;
		}
		
		private void close() {
			try {
				this.channel.close();
			} catch (IOException e) { }
		}

		/**
         * Helper method that receives a single message from the socket channel.
         * Will demarshall the message and schedule the message with the TCP
         * program as a {@link TCPNIO#ReceiveMessage} tuple.
         */
		private void receive() {
			try {
				Message message = null;
				synchronized (buffer) {
					this.buffer.clear();
					this.buffer.limit(Integer.SIZE / Byte.SIZE);
					int bytes = read(this.buffer, this.channel, true);
					if (bytes == 0) return;

					int length = this.buffer.getInt(0);
					this.buffer.clear();
					this.buffer.limit(length);
					read(this.buffer, this.channel, false);

					ObjectInputStream istream = new ObjectInputStream(new ByteArrayInputStream(this.buffer.array()));
					message = (Message) istream.readObject();
				}
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
				try {
					TCPNIO.this.manager.connection().unregister(this);
				} catch (UpdateException e1) {
					e1.printStackTrace();
					System.exit(0);
				}
			}
		}
		private int totalWritten = 0;
		/**
		 * Method will write ALL data contained in the byte buffer.
		 * This method will not return until the entire buffer has been written
		 * to the socket channel.
		 * @param buf The data buffer.
		 * @param socket The socket channel.
		 * @throws IOException
		 */
	    private void write(ByteBuffer buf, SocketChannel socket) throws IOException {
	        int len = buf.limit() - buf.position();
	        totalWritten += len;
	        
	        while (len > 0) {
	            len -= socket.write(buf);
	        }
	        
	        System.out.println("TCPNIO: Wrote " + totalWritten + " bytes so far");
	    }
	 
	    /**
	     * Read data from the socket channel and place it into the buffer. The 
	     * amount that is to be read is taken from the current buffer position
	     * and limit. That is, length_to_read = buf.limit() - buf.position().
	     * This method will continue to read from the socket until this amount of
	     * data has been received UNLESS the third parameter is set to true.
	     * @param buf The buffer that will contain the data after this call.
	     * @param socket The socket channel that will be read.
	     * @param once If true then only 1 attempt will be made to read from the socket,
	     * otherwise an infinite number of attempts will be made to read (buf.limit() - buf.position())
	     * bytes from the socket channel.
	     * @return The number of bytes read.
	     * @throws IOException
	     */
	    private int read(ByteBuffer buf, SocketChannel socket, boolean once) throws IOException {
	    	int read = 0;
	    	int attempts = 0;
	        int total = buf.limit() - buf.position();

	        do {
	            int bytes = socket.read(buf);
	            if (bytes == 0) attempts++;
	            else attempts = 0;
	            
	            if (attempts > MAX_SOCKET_ATTEMPTS) {
	            	throw new IOException("TCPNIO: read from socket " + attempts + " times with no progress");
	            }
	            read += bytes;
	        } while (!once && read < total);

	        return read;
	    }
	}

}
