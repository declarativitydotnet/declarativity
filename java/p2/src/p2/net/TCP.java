package p2.net;

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

public class TCP extends ObjectTable {
	
	private static class TCPChannel extends Channel {
		
		private Socket socket;
		
		public TCPChannel(String host, Integer port) throws Exception {
			super(host + ":" + port);
			this.socket = new Socket(host, port);
		}

		@Override
		public boolean send(TupleSet tuples) {
			// TODO Auto-generated method stub
			return false;
		}
		
	}
	
	public static final Key PRIMARY_KEY = new Key(0,1);
	
	public enum Field{DESTINATION, PORT, CHANNEL};
	public static final Class[] SCHEMA = {
		String.class,   // Destination Location
		Integer.class,  // Destination Port 
		Channel.class   // Channel
	};
	
	public TCP(String hostname, Integer port) {
		super(new TableName("network", "tcp"), PRIMARY_KEY, new TypeList(SCHEMA));
	}
	
	
	public TupleSet insert(TupleSet tuples) {
		TupleSet delta = new TupleSet(tuples.name());
		
		for (Tuple tuple : tuples) {
			String host = (String) tuple.value(Field.DESTINATION.ordinal());
			Integer port = (Integer) tuple.value(Field.PORT.ordinal());
		}
		
		return delta;
	}

}
