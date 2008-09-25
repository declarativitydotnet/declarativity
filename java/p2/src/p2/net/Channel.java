package p2.net;

import p2.types.basic.TupleSet;

public abstract class Channel implements Comparable<Channel> {
	
	private String protocol;
	
	private String address;
	
	protected Channel(String protocol, String address) {
		this.protocol = protocol;
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
	
	public String protocol() {
		return this.protocol;
	}
	
	public String address() {
		return this.address;
	}
	
	public abstract boolean send(Message message);
	
	public abstract void close();

}
