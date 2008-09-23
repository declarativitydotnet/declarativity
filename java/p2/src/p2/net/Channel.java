package p2.net;

import p2.types.basic.TupleSet;

public abstract class Channel implements Comparable<Channel>, Runnable {
	
	private String address;
	
	protected Channel(String address) {
		this.address = address;
	}

	@Override
	public boolean equals(Object o) {
		if (o instanceof Channel) {
			return compareTo((Channel)o) == 0;
		}
		return false;
	}
	
	public int compareTo(Channel o) {
		return this.address.compareTo(o.address);
	}
	
	public String address() {
		return this.address;
	}
	
	public abstract boolean send(Packet packet);
	
	public abstract void close();

}
