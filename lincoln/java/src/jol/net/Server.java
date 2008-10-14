package jol.net;

public abstract class Server extends Thread {

	public Server(String name) {
		super(name);
		super.setDaemon(true);
	}
	
	/**
	 * Open a new channel.
	 * @param address The channel address.
	 * @return A channel object.
	 */
	public abstract Channel open(Address address);
	
	/**
	 * Close the channel.
	 * @param channel The channel to close.
	 */
	public abstract void close(Channel channel);
}
