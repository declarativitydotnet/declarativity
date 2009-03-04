package jol.net;

import java.io.Serializable;

/**
 * The interface class for messages.
 */
public abstract class Message implements Comparable<Message>, Serializable {
	private static final long serialVersionUID = 1L;

	private Long id;

	private String layer;

	public Message() {
		this.id = 0L;
	}

	public Message(Long id, String layer) {
		this.id = id;
		this.layer = layer;
	}

	@Override
	public String toString() {
		return layer + ":" + id.toString();
	}

	public int compareTo(Message o) {
		return toString().compareTo(o.toString());
	}

	public Long id() {
		return this.id;
	}

	public String layer() {
		return this.layer;
	}
}
