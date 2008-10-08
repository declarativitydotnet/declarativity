package jol.net;

public abstract class Channel implements Comparable<Channel> {
	
	private String protocol;
	
	private Address address;
	
	protected Channel(String protocol, Address address) {
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
	
	public Address address() {
		return this.address;
	}
	
	public abstract boolean send(Message message);
	
	public abstract void close();

}
