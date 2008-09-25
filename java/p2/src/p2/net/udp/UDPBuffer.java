package p2.net.udp;

import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

public class UDPBuffer extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key();
	
	public enum Field{DIRECTION, ADDRESS, ID, OFFSET, FRAGMENTS, PAYLOAD};
	public static final Class[] SCHEMA = {
		String.class,  // send/receive
		String.class,  // address (source) 
		Long.class,    // Message ID
		Integer.class, // Offset
		Integer.class, // Fragments
		UDPByteBuffer.class // Payload
	};
	
	public UDPBuffer() {
		super(new TableName("network", "udpbuffer"), PRIMARY_KEY, new TypeList(SCHEMA));
	}


}
