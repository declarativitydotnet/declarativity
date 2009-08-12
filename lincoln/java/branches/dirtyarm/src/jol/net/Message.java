package jol.net;

import java.io.Serializable;

/**
 * The interface class for messages.
 */
public abstract class Message implements Serializable {
	private static final long serialVersionUID = 1L;

	private String layer;

	public Message(String layer) {
		this.layer = layer;
	}

	public String layer() {
		return this.layer;
	}
	
	public abstract String toString();
}
