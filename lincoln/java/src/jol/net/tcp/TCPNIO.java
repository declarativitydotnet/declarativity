package jol.net.tcp;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.channels.spi.SelectorProvider;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

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
	
	private Runtime context;
	
	private Network manager;
	
	private Selector selector;
	
	private List<Connection> newConnections;

	public TCPNIO(Runtime context, Network manager, Integer port) throws IOException, UpdateException {
		super("TCPNIO Server");
		this.context = context;
		this.manager = manager;
		this.selector = SelectorProvider.provider().openSelector();
		this.newConnections = new ArrayList<Connection>();
		context.install("system", "jol/net/tcp/tcp.olg");
		
		ServerSocketChannel server = ServerSocketChannel.open();
		server.configureBlocking(false);
		server.socket().bind(new InetSocketAddress(port));
		server.register(this.selector, SelectionKey.OP_ACCEPT);
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
				
		        /* Iterate over the keys for which events are available */
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
							Connection conn = new Connection(channel);
							manager.connection().register(conn);
							register(conn);
						}
					}
					else if (key.isReadable()) {
						Connection conn = (Connection) key.attachment();
                        conn.read();
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
			Connection conn = new Connection(channel);
			register(conn);
			return conn;
		} catch (Exception e) {
			return null;
		}
	}
	
	@Override
	public void close(Channel channel) {
		if (!(channel instanceof Connection))
			throw new IllegalArgumentException("Unrecognized TCPNIO channel: " +
											   channel.toString());

		Connection conn = (Connection) channel;
		conn.close();
	}
	
	private void register(Connection connection) {
		synchronized (this.newConnections) {
			this.newConnections.add(connection);
			this.selector.wakeup();
		}
	}
	
	private class Connection extends Channel {
        private static final int LENGTH_WORD_SIZE = Integer.SIZE / Byte.SIZE;
        private static final int READ_DONE = 1;
        private static final int READ_LENGTH = 2;
        private static final int READ_MESSAGE = 3;

		private ByteBuffer buffer;
		private SocketChannel channel;
        private String remoteAddr;
        private int readState;

		public Connection(SocketChannel channel) throws IOException {
			super("tcp", new IP(channel.socket().getInetAddress(), channel.socket().getPort()));
			this.buffer = ByteBuffer.allocate(8192);
			this.channel = channel;
			this.channel.configureBlocking(false);
            this.remoteAddr = channel.socket().toString();
            this.readState = READ_DONE;
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
					this.buffer.putInt(bstream.size());
					this.buffer.put(bstream.toByteArray());
					this.buffer.flip();
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
			} catch (IOException e) {
                throw new RuntimeException(e);
			}
		}

        /**
         * There is some input data available to be read, so try to read
         * it. This advances the read state machine appropriately, depending on
         * how much input is actually available.
         */
        private void read() {
            try {
                switch (this.readState) {
                case READ_DONE:
                    this.buffer.clear();
                    this.buffer.limit(LENGTH_WORD_SIZE);
                    this.readState = READ_LENGTH;
                    /* fallthrough and try to read length */

                case READ_LENGTH:
                    doSocketRead();
                    if (this.buffer.hasRemaining())
                        return;

                    int msgLength = this.buffer.getInt(0);
                    this.buffer.clear();
                    this.buffer.limit(msgLength);
                    this.readState = READ_MESSAGE;
                    /* fallthrough and try to read message */

                case READ_MESSAGE:
                    doSocketRead();
                    if (this.buffer.hasRemaining())
                        return;

                    constructReceiveMessage();
                    this.buffer.clear();
                    this.readState = READ_DONE;
                }
            } catch (IOException e) {
                if (e instanceof ClosedChannelException)
                    java.lang.System.err.println("TCP channel closed: " +
                                                 this.remoteAddr);
                else
                    java.lang.System.err.println("Unexpected IO exception: " +
                                                 this.remoteAddr);

                try {
                    TCPNIO.this.manager.connection().unregister(this);
                } catch (UpdateException ue) {
                    throw new RuntimeException(ue);
                }
            }
        }

        private void doSocketRead() throws IOException {
            int bytes = this.channel.read(this.buffer);
            if (bytes == -1)
                throw new ClosedChannelException();
        }

        /**
         * Demarshall an already-read message from the input buffer, and
         * schedule the message with the TCP program as a {@link
         * TCPNIO#ReceiveMessage} tuple.
         */
        private void constructReceiveMessage() throws IOException {
            try {
                ObjectInputStream istream = new ObjectInputStream(new ByteArrayInputStream(this.buffer.array()));
                Message message = (Message) istream.readObject();
                IP address = new IP(this.channel.socket().getInetAddress(), this.channel.socket().getPort());
                Tuple tuple = new Tuple(address, message);
                context.schedule("tcp", ReceiveMessage, new TupleSet(ReceiveMessage, tuple), null);
            } catch (ClassNotFoundException e) {
                throw new RuntimeException(e);
            } catch (UpdateException e) {
                throw new RuntimeException(e);
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
	}
}
