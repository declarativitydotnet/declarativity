package p2.net;

import p2.lang.plan.Fact.FactTable.Field;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.table.HashIndex;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

public class NetworkBuffer extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key(0,1,2);
	
	public enum Field{PROTOCOL, ADDRESS, PROGRAM, INSERTION, DELETION};
	public static final Class[] SCHEMA = {
		String.class,   // PROTOCOL
		String.class,   // Destination address 
		String.class,   // Program
		TupleSet.class, // Insertion Payload
		TupleSet.class  // Deletion  Payload
	};
	
	public NetworkBuffer() {
		super(new TableName("network", "buffer"), PRIMARY_KEY, new TypeList(SCHEMA));
	}

}
