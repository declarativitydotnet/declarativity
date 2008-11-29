package jol.net;

import java.net.InetAddress;
import java.net.UnknownHostException;

public class IP extends Address {
	
	private InetAddress address;
	
	private Integer port;
	
	public IP(String address) {
		try {
			this.address = InetAddress.getByName(address.substring(0, address.indexOf(':')));
			this.port    = Integer.parseInt(address.substring(address.indexOf(':') + 1));
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public IP(InetAddress address, Integer port) {
		this.address = address;
		this.port = port;
	}

	@Override
	public String toString() {
		return address.toString() + ":" + port;
	}
	
	public InetAddress address() {
		return this.address;
	}
	
	public Integer port() {
		return this.port;
	}
}
