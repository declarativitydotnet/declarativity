package p2.core;

import p2.exec.Query;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;

public class Clock extends ObjectTable {
	public static final Key PRIMARY_KEY = new Key(0);
	
	public enum Field{LOCATION, CLOCK};
	public static final Class[] SCHEMA = {
		String.class,  // Location
		Long.class     // Clock value 
	};
	
	private String location;
	
	private Long clock;

	public Clock(String location) {
		super("clock", PRIMARY_KEY, new TypeList(SCHEMA));
		this.location = location;
		this.clock = -1L;
		try {
			set(0L);
		} catch (UpdateException e) {
			e.printStackTrace();
			java.lang.System.exit(0);
		}
	}
	
	public Long current() {
		return this.clock;
	}
	
	public TupleSet set(Long clock) throws UpdateException {
		if (clock > this.clock) {
			this.clock = clock;
			force(new Tuple(name(), location, clock));
			return this.tuples;
		}
		return null;
	}
}
