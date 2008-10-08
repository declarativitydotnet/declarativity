package p2.net.udp;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;

import p2.net.Address;
import p2.net.Channel;
import p2.net.IP;
import p2.net.Message;
import p2.net.Server;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.UpdateException;
import p2.types.table.TableName;
import p2.core.Runtime;

public class UDP extends Server {
	private static final TableName UDPMessage = new TableName("udp", "message");
	
	public static final int max_packet  = 1500;
	public static final int max_payload = 1400;
	
	private Runtime context;
	
	private DatagramSocket server;
		
	public UDP(Runtime context, Integer port) throws IOException, UpdateException {
		this.context = context;
		this.server = new DatagramSocket(port);
		context.install("system",
				ClassLoader.getSystemClassLoader().getResource("p2/net/udp/udp.olg"));
	}
	
	/**
	 * Receives UDP packets and adds them to the UDP Buffer table.
	 */
	public void run() {
		while (true) {
			try {
				byte[] buf = new byte[max_packet];
				DatagramPacket packet = new DatagramPacket(buf, buf.length);
				server.receive(packet);
				
				try {
					ObjectInputStream input = new ObjectInputStream(new ByteArrayInputStream(buf));
					Message message = (Message) input.readObject();
					Tuple tuple = new Tuple("receive", new IP(packet.getAddress(), packet.getPort()), message);
					context.schedule("tcp", UDPMessage, new TupleSet(UDPMessage, tuple), new TupleSet(UDPMessage));
				} catch (ClassNotFoundException e) {
					e.printStackTrace();
					System.exit(0);
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
	@Override
	public Channel open(Address address) {
		try {
			return new Connection(address);
		} catch (Exception e) {
			return null;
		}
	}
	
	@Override
	public void close(Channel channel) {
		channel.close();
	}
	
	private static class Connection extends Channel {
		private DatagramSocket socket;
		
		private IP address;
		
		public Connection(Address address) throws SocketException { 
			super("udp", address);
			this.socket = new DatagramSocket();
			this.address = (IP) address;
		}
		
		@Override
		public Address address() {
			return this.address;
		}
		
		@Override
		public boolean send(Message message) {
			try {
				ByteArrayOutputStream data = new ByteArrayOutputStream(UDP.max_packet);
				ObjectOutputStream oss = new ObjectOutputStream(data);
				oss.writeObject(message);
				if (data.size() > UDP.max_packet) {
					System.err.println("UDP packet exceeds maximum allowable size!");
					return false;
				}
				
				DatagramPacket dgram = new DatagramPacket(data.toByteArray(), data.size());
				this.socket.send(dgram);
			} catch (Exception e) {
				e.printStackTrace();
				return false;
			}
			return true;
		}
		
		@Override
		public void close() {
			socket.close();
		}
	}

}
