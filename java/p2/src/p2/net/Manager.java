package p2.net;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.util.Hashtable;


public final class Manager {
	private static final NetworkBuffer buffer = new NetworkBuffer();
	
	private Connection connection;
	
	private Hashtable<String, Server> servers; 
	
	private Hashtable<String, Thread> serverThreads; 
	
	public Manager() throws IOException {
		this.connection    = new Connection(this);
		this.servers       = new Hashtable<String, Server>();
		this.serverThreads = new Hashtable<String, Thread>();
	}
	
	public final static NetworkBuffer buffer() {
		return buffer;
	}
	
	public void server(String protocol, Server server) {
		Thread thread = new Thread(server);
		thread.start();
		this.servers.put(protocol, server);
		this.serverThreads.put(protocol, thread);
	}
	
	public Channel create(String protocol, String address) {
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
