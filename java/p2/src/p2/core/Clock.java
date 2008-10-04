package p2.core;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

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
		super(new TableName(GLOBALSCOPE, "clock"), PRIMARY_KEY, new TypeList(SCHEMA));
		this.location = location;
		this.clock = 0L;
	}
	
	public Long current() {
		return this.clock;
	}
	
	public TupleSet time(Long time) {
		return new TupleSet(name(), new Tuple(location, time));
	}
	
	public boolean insert(Tuple tuple) throws UpdateException {
		Long time = (Long) tuple.value(Field.CLOCK.ordinal());
		if (time < this.clock) {
			throw new UpdateException("Invalid clock time " +  time + 
					                  " current clock value = " + this.clock);
		}
		this.clock = time;
		return super.insert(tuple);
	}
	
	public boolean delete(Tuple tuple) throws UpdateException {
		return super.delete(tuple);
	}
}
