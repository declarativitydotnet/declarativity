package jol.net;

public abstract class Server implements Runnable {

	public abstract Channel open(Address address);
	
	public abstract void close(Channel channel);
}
