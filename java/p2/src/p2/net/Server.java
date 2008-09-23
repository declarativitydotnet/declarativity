package p2.net;

public abstract class Server implements Runnable {

	public abstract Channel channel(String address);
}
