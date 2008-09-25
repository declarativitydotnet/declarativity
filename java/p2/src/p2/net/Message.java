package p2.net;

import java.io.Serializable;

public class Message implements Serializable {
	private static final long serialVersionUID = 1L;
	
	public Long id;
	
	public Message() {
		this.id = 0L;
	}
	
	public Message(Long id) {
		this.id = id;
	}

}
