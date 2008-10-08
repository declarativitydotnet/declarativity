package jol.net;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;
import jol.core.Runtime;

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
