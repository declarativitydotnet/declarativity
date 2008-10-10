package jol.net;

import java.io.IOException;
import java.util.Hashtable;

import jol.net.tcp.TCP;
import jol.types.basic.Tuple;
import jol.types.exception.UpdateException;
import jol.core.Runtime;

public final class Network {
	private Runtime context;
	
	private NetworkBuffer buffer;
	
	private Connection connection;
	
	private Hashtable<String, Server> servers; 
	
	private Hashtable<String, Thread> serverThreads; 
	
	public Network(Runtime context) {
		this.context       = context;
		this.buffer        = new NetworkBuffer(context);
		this.connection    = new Connection(context, this);
		this.servers       = new Hashtable<String, Server>();
		this.serverThreads = new Hashtable<String, Thread>();
		
		context.catalog().register(this.buffer);
		context.catalog().register(this.connection);
	}
	
	public final void install(Integer port) throws IOException, UpdateException {
		/* Install protocols */
		server("tcp", new TCP(context, this, port));
		// server("udp", new UDP(port+1));
		
		/* Install network layer application */
		context.install("network",
				ClassLoader.getSystemClassLoader().getResource("jol/net/network.olg"));
	}
	
	public final static Tuple tuple(String direction, Address address, Message message) {
		return new Tuple(direction, address, message);
	}
	
	public NetworkBuffer buffer() {
		return this.buffer;
	}
	
	public void server(String protocol, Server server) {
		Thread thread = new Thread(server);
		thread.start();
		this.servers.put(protocol, server);
		this.serverThreads.put(protocol, thread);
	}
	
	public Channel create(String protocol, Address address) {
		if (!servers.containsKey(protocol)) {
			System.err.println("ERROR: Unknown protocol " + protocol);
			return null;
		}
		
		try {
			Channel channel = servers.get(protocol).open(address);
			return channel;
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}
	
	/**
	 * Get the connection table. The purpose of this
	 * method is for the TCP server to register/unregister new connections.
	 * @return The connection table.
	 */
	public Connection connection() {
		return this.connection;
	}
	
	/**
	 * Close the connection channel and stop its thread.
	 * @param channel Connection channel to be stopped. 
	 */
	public void close(Channel channel) {
		if (!servers.containsKey(channel.protocol())) {
			System.err.println("ERROR: Unknown protocol " + channel.protocol());
			return;
		}
		
		servers.get(channel.protocol()).close(channel);
	}
}
