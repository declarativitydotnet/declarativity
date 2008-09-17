package p2.net;

import p2.types.basic.TupleSet;

public abstract class Channel implements Comparable<Channel> {
	
	private String id;
	
	protected Channel(String id) {
		this.id = id;
	}

	public int compareTo(Channel o) {
		return this.id.compareTo(o.id);
	}
	
	public abstract boolean send(TupleSet tuples);

}
