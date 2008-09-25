package p2.net.udp;

import p2.net.Message;

public class UDPPacket extends Message {
	public int offset;
	
	public int fragments;
	
	public UDPByteBuffer payload;
	
	public UDPPacket() {
	}
	
	public UDPPacket(long id, int offset, int fragments, UDPByteBuffer payload) {
		super(id);
		this.offset    = offset;
		this.fragments = fragments;
		this.payload   = payload;
	}
}
