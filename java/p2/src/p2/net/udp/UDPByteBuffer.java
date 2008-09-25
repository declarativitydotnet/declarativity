package p2.net.udp;

import java.io.Serializable;

public class UDPByteBuffer implements Serializable, Comparable<UDPByteBuffer> {
	
	public byte[] buffer;
	
	public UDPByteBuffer() {
		buffer = new byte[UDP.max_payload];
	}
	
	public UDPByteBuffer(byte[] buffer) {
		if (buffer.length > UDP.max_payload) {
			System.err.println("BYTE BUFFER TOO LARGE!");
			System.exit(0);
		}
		this.buffer = buffer;
	}

	public int compareTo(UDPByteBuffer o) {
		return buffer.hashCode() == o.buffer.hashCode() ? 0 : -1;
	}

}
