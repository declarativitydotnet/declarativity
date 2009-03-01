package jol.core;

import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;
import jol.core.Runtime;

/**
 * The system clock. 
 * A single clock tick represents a full fixpoint iteration
 * of all programs currently installed in the Runtime.
 *
 */
public class Clock extends ObjectTable {
	/** The table name */
	public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "clock");
	
	/** The primary key */
	public static final Key PRIMARY_KEY = new Key(0);
	
	/** An enumeration of all clock table fields. */
	public enum Field{LOCATION, CLOCK};
	
	/** The table schema types. */
	public static final Class[] SCHEMA = {
		String.class,  // Location
		Long.class     // Clock value 
	};
	
	/** The location of this clock */
	private String location;
	
	/** The clock value. */
	private Long clock;

	/**
	 * Creates a new clock.
	 * @param context The runtime context
	 * @param location The location of the clock (e.g., localhost)
	 */
	public Clock(Runtime context, String location) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
		this.location = location;
		this.clock = 0L;
	}
	
	/**
	 * @return The current clock value.
	 */
	public Long current() {
		return this.clock;
	}
	
	/**
	 * Creates a tupleset containing the given time value.
	 * @param time The time value.
	 * @return A tupleset that can be inserted into the clock table.
	 */
	public BasicTupleSet time(Long time) {
		return new BasicTupleSet(name(), new Tuple(location, time));
	}
	
	/**
	 * Updates the clock to the time indicated in the tuple.
	 * @param tuple Contains the new clock value. 
	 */
	@Override
	public boolean insert(Tuple tuple) throws UpdateException {
		Long time = (Long) tuple.value(Field.CLOCK.ordinal());
		if (time < this.clock) {
			throw new UpdateException("Invalid clock time " +  time + 
					                  " current clock value = " + this.clock);
		}
		this.clock = time;
		return super.insert(tuple);
	}
	
	@Override
	public boolean delete(Tuple tuple) throws UpdateException {
		return super.delete(tuple);
	}
}
