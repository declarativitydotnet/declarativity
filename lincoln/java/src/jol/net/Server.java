package jol.net;

public abstract class Server extends Thread {

	public abstract Channel open(Address address);
	
	public abstract void close(Channel channel);
	
}
