package p2.core;

import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;

public class Periodic extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key(0);
	
	public enum Field {IDENTIFIER, PERIOD, TTL, COUNT};
	public static final Class[] SCHEMA = { 
		String.class, // Identifier
		Long.class,   // Period
		Long.class,   // TTL
		Long.class,   // Count
	};

	protected Periodic() {
		super("periodic", PRIMARY_KEY, new TypeList(SCHEMA));
	}
	
	protected boolean insert(Tuple tuple) throws UpdateException {
		Long ttl   = (Long) tuple.value(Field.TTL.ordinal());
		Long count = (Long) tuple.value(Field.COUNT.ordinal());
		if (count < ttl) {
			tuple.value(Field.COUNT.ordinal(), count + 1L);
			return super.insert(tuple);
		}
		return false;
	}
}
