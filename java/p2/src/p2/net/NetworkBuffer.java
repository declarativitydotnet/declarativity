package p2.net;

import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

public class NetworkBuffer extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key();
	
	public enum Field{DESTINATION, PAYLOAD};
	public static final Class[] SCHEMA = {
		String.class,   // Destination Location 
		TupleSet.class  // Payload
	};
	
	public NetworkBuffer() {
		super(new TableName("network", "buffer"), PRIMARY_KEY, new TypeList(SCHEMA));
	}

}
