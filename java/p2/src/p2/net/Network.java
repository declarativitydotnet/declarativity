package p2.net;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.util.Hashtable;

import p2.net.tcp.TCP;
import p2.net.udp.UDP;
import p2.types.basic.Tuple;
import p2.types.exception.UpdateException;
import p2.types.table.TableName;


public final class Network {
	private static final NetworkBuffer buffer = new NetworkBuffer();
	
	private Connection connection;
	
	private Hashtable<String, Server> servers; 
	
	private Hashtable<String, Thread> serverThreads; 
	
	public Network() throws IOException {
		this.connection    = new Connection(this);
		this.servers       = new Hashtable<String, Server>();
		this.serverThreads = new Hashtable<String, Thread>();
	}
	
	public final void install(Integer port) throws IOException, UpdateException {
		/* Install protocols */
		server("tcp", new TCP(this, port));
		/*
		server("udp", new UDP(this, port+1));
		*/
		
		/* Install network layer application */
		p2.core.System.install("network",
				ClassLoader.getSystemClassLoader().getResource("p2/net/network.olg").getPath());
	}
	
	public final static Tuple tuple(String direction, Address address, Message message) {
		return new Tuple(direction, address, message);
	}
	
	public final static TableName bufferName() {
		return buffer.name();
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
