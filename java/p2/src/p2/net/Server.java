package p2.net;

public abstract class Server implements Runnable {

	public abstract Channel open(String address);
	
	public abstract void close(Channel channel);
}
