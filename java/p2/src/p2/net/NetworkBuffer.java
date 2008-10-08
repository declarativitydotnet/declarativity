package p2.net;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;
import p2.core.Runtime;

public class NetworkBuffer extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key(2,3);
	
	public enum Field{PROTOCOL, DIRECTION, ADDRESS, MESSAGE};
	public static final Class[] SCHEMA = {
		String.class,   // Protocol
		String.class,   // Direction
		Address.class,  // Destination address 
		Message.class   // Message
	};
	
	public NetworkBuffer(Runtime context) {
		super(context, new TableName("network", "buffer"), PRIMARY_KEY, new TypeList(SCHEMA));
	}
	
	public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
		return super.insert(tuples, conflicts);
	}
	
	@Override
	public boolean insert(Tuple tuple) throws UpdateException {
		return super.insert(tuple);
	}
	
	@Override
	public boolean delete(Tuple tuple) throws UpdateException {
		return super.delete(tuple);
	}

}
