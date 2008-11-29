package jol.net;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import jol.core.Runtime;
import jol.net.tcp.TCPNIO;
import jol.types.basic.Tuple;
import jol.types.exception.UpdateException;

/**
 * The Network manager class. 
 * 
 * <p>
 * The network manager is responsible for the initialization and
 * maintenance of all networking components.
 */
public final class Network {
	private Runtime context;
	
	private NetworkBuffer buffer;
	
	private Connection connection;
	
	private Map<String, Server> servers; 
	
	/**
	 * Create a new network manager.
	 * This will allocate a network buffer table {@link NetworkBuffer} and a
	 * connection table {@link Connection}.
	 * @param context The runtime context.
	 */
	public Network(Runtime context) {
		this.context       = context;
		this.buffer        = new NetworkBuffer(context);
		this.connection    = new Connection(context, this);
		this.servers       = new HashMap<String, Server>();
		
		context.catalog().register(this.buffer);
		context.catalog().register(this.connection);
	}
	
	/**
	 * Install network components.
	 * @param port The listen port.
	 * @throws IOException
	 * @throws UpdateException
	 */
	public final void install(Integer port) throws IOException, UpdateException {
		/* Install protocols */
		server("tcp", new TCPNIO(context, this, port));
		// server("udp", new UDP(port+1));
		
		/* Install network layer application */
		context.install("network", "jol/net/network.olg");
	}
	
	/**
	 * Shutdown all network connections.
	 */
	public final void shutdown() {
		for (Server server : servers.values()) {
			server.interrupt();
      server.cleanup();
		}
		try {
			this.connection.delete(this.connection.tuples());
			this.buffer.delete(this.buffer.tuples());
		} catch (UpdateException e) {
			// TODO Log error.
		}
	}
	
	/**
	 * Method for creating a tuple that can be inserted into the network buffer.
	 * @param direction The message direction (send/receive)
	 * @param address The message address.
	 * @param message The message payload.
	 * @return A tuple filled in with formal values.
	 */
	public final static Tuple tuple(String direction, Address address, Message message) {
		return new Tuple(direction, address, message);
	}
	
	/**
	 * Get the network buffer.
	 * @return The network buffer object.
	 */
	public NetworkBuffer buffer() {
		return this.buffer;
	}
	
	/**
	 * Start and register a new protocol server.
	 * @param protocol The protocol name.
	 * @param server The server.
	 */
	private void server(String protocol, Server server) {
		server.start();
		this.servers.put(protocol, server);
	}
	
	/**
	 * Create a new channel.
	 * NOTE: Will not register channel with {@link Connection} table.
	 * @param protocol The channel protocol.
	 * @param address The channel address.
	 * @return A new channel object or null on error.
	 */
	public Channel create(String protocol, Address address) {
		if (!servers.containsKey(protocol)) {
			System.err.println("ERROR: Unknown protocol " + protocol);
			return null;
		}

		return servers.get(protocol).open(address);
	}
	
	/**
	 * Close the connection channel and stop its thread.
	 * This method will NOT remove the channel from the
	 * connection table. That must be done explicitly by
	 * either deleting from the {@link #Connection} table
	 * or calling the {@link Connection#unregister(Channel)}
	 * method.
	 * @param channel Connection channel to be stopped. 
	 */
	public void close(Channel channel) {
		if (!servers.containsKey(channel.protocol())) {
			System.err.println("ERROR: Unknown protocol " + channel.protocol());
			return;
		}
		servers.get(channel.protocol()).close(channel);
	}
	
	/**
	 * Get the connection table. The purpose of this method is to
	 * allow protocol servers to register and unregister connections.
	 * @return The connection table.
	 */
	public Connection connection() {
		return this.connection;
	}
}
