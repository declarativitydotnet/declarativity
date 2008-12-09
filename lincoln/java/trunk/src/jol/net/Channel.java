package jol.net;

/**
 * An interface to all channels.
 *
 * All channels must provide to ability to send a {@link Message}.
 */
public abstract class Channel implements Comparable<Channel> {

	private String protocol;

	private Address address;

	/**
	 * Create a new channel.
	 * @param protocol The channel protocol.
	 * @param address The remote address.
	 */
	protected Channel(String protocol, Address address) {
		this.protocol = protocol;
		this.address = address;
	}

	@Override
	public boolean equals(Object o) {
		if (o instanceof Channel) {
			return compareTo((Channel) o) == 0;
		}
		return false;
	}

	/**
	 * Compare channels based on address {@link #address()}.
	 */
	public int compareTo(Channel o) {
		return this.address.compareTo(o.address);
	}

	@Override
    public int hashCode() {
	    return this.address.hashCode();
    }

    /**
	 * The channel protocol name.
	 * @return The channel protocol.
	 */
	public String protocol() {
		return this.protocol;
	}

	/**
	 * The address of the remote location that connects
	 * to this channel.
	 * @return An address with the remote location.
	 */
	public Address address() {
		return this.address;
	}

	public abstract boolean send(Message message);
}
