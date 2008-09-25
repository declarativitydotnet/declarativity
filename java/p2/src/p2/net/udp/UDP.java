package p2.net.udp;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;

import p2.net.Channel;
import p2.net.Manager;
import p2.net.Application;
import p2.net.Message;
import p2.net.Server;
import p2.types.basic.Tuple;
import p2.types.exception.UpdateException;

public class UDP extends Server {
	public static final int max_packet  = 1500;
	public static final int max_payload = 1400;
	
	private static final UDPBuffer buffer = new UDPBuffer();
	
	private Manager manager;
	
	private DatagramSocket server;
		
	public UDP(Manager manager, Integer port) throws IOException {
		this.manager = manager;
		this.server = new DatagramSocket(port);
	}
	
	/**
	 * Receives UDP packets and adds them to the UDP Buffer table.
	 */
	public void run() {
		while (true) {
			try {
				byte[] buf = new byte[max_packet];
				DatagramPacket dpacket = new DatagramPacket(buf, buf.length);
				server.receive(dpacket);
				
				ObjectInputStream input = new ObjectInputStream(new ByteArrayInputStream(buf));
				try {
					UDPPacket udpPacket = (UDPPacket) input.readObject();
					Tuple tuple = new Tuple(dpacket.getAddress().toString(), udpPacket.id, 
							                udpPacket.offset, udpPacket.fragments, udpPacket.payload);
					buffer.force(tuple);
				} catch (ClassNotFoundException e) {
					e.printStackTrace();
					System.exit(0);
				} catch (UpdateException e) {
					e.printStackTrace();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
	@Override
	public Channel open(String address) {
		try {
			return new Connection(address);
		} catch (Exception e) {
			return null;
		}
	}
	
	@Override
	public void close(Channel channel) {
		// TODO Auto-generated method stub
	}
	
	private static class Connection extends Channel {
		private long ids;
		
		private DatagramSocket socket;
		
		private String host;
		
		private Integer port;

		public Connection(String address) throws SocketException { 
			super("udp", address);
			this.ids    = 0;
			this.socket = new DatagramSocket();
			this.host   = address.substring(0, address.indexOf(':'));
			this.port   = Integer.parseInt(address.substring(address.indexOf(':')+1));
		}
		
		@Override
		public String address() {
			return this.socket.getInetAddress().toString();
		}
		
		@Override
		public boolean send(Message message) {
			try {
				if (message instanceof UDPPacket) {
					send(message);
				}
				else {
					ByteArrayOutputStream obstream = new ByteArrayOutputStream(UDP.max_payload);
					ObjectOutputStream    ostream = new ObjectOutputStream(obstream);
					ostream.writeObject(message);

					byte [] payload = obstream.toByteArray();
					if (payload.length > UDP.max_payload) {
						int fragments = (payload.length / UDP.max_payload) + 1;
						for (int offset = 0; offset < fragments; offset++) {
							byte [] frag = new byte[UDP.max_payload];
							System.arraycopy(payload, offset*UDP.max_payload, frag, 0, UDP.max_payload);
							buffer(ids, offset, fragments, new UDPByteBuffer(frag));
						}
						this.ids++;
					}
					else {
						buffer(ids++, 0, 1, new UDPByteBuffer(payload));
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
				return false;
			}
			return true;
		}
		
		private void buffer(long id, int offset, int fragments, UDPByteBuffer payload) throws UpdateException {
			Tuple tuple = new Tuple("send", address(), id, offset, fragments, payload);
			UDP.buffer.force(tuple);
		}
		
		private void send(UDPPacket packet) throws IOException {
			ByteArrayOutputStream data = new ByteArrayOutputStream(UDP.max_packet);
			ObjectOutputStream oss = new ObjectOutputStream(data);
			oss.writeObject(packet);
			DatagramPacket dgram = new DatagramPacket(data.toByteArray(), data.size());
			this.socket.send(dgram);
		}
		
		@Override
		public void close() {
			socket.close();
		}
	}

}
