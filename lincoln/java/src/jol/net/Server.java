package jol.net;

public abstract class Server extends Thread {

	public Server(String name) {
		super(name);
		super.setDaemon(true);
	}
	
	public abstract Channel open(Address address);
	
	public abstract void close(Channel channel);
	
}
