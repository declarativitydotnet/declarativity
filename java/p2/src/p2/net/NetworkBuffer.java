package p2.net;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

public class NetworkBuffer extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key(2,3);
	
	public enum Field{PROTOCOL, DIRECTION, ADDRESS, MESSAGE};
	public static final Class[] SCHEMA = {
		String.class,   // Protocol
		String.class,   // Direction
		Address.class,  // Destination address 
		Message.class   // Message
	};
	
	public NetworkBuffer() {
		super(new TableName("network", "buffer"), PRIMARY_KEY, new TypeList(SCHEMA));
	}
	
	public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
		return super.insert(tuples, conflicts);
	}
	
	@Override
	public boolean insert(Tuple tuple) throws UpdateException {
		if (super.insert(tuple)) {
			System.err.println("INSSERTION SUCCESS: " + tuple);
			return true;
		}
		else {
			System.err.println("INSERTION FAIL " + tuple);
			return false;
		}
	}
	
	@Override
	public boolean delete(Tuple tuple) throws UpdateException {
		if (super.delete(tuple)) {
			System.err.println("DELETION SUCCESS: " + tuple);
			return true;
		}
		else {
			System.err.println("DELETION FAIL " + tuple);
			return false;
		}
	}

}
