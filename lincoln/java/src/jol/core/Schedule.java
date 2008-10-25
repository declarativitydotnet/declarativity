package jol.core;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

/**
 * Stores the set of tuples that are waiting to be 
 * inserted/deleted from the table store. The delta
 * tuples from the insert/delete operation will subsequently
 * be evaluated by the query processor {@link Driver} and
 * the result of that evaluation is expected to be placed
 * back into this table.
 *
 */
public class Schedule extends ObjectTable {
	
	/** The name of the schedule table. */
	public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "schedule");
	
	/** The primary key. */
	public static final Key PRIMARY_KEY = new Key();
	
	/** The field names of each attribute. */
	public enum Field {TIME, PROGRAM, TABLENAME, INSERTIONS, DELETIONS};
	
	/** The type of each attribute. */
	public static final Class[] SCHEMA = { 
		Long.class,      // Time
		String.class,    // Program name
		TableName.class, // Table name
		TupleSet.class,  // Insertion tuple set
		TupleSet.class   // Deletion tuple set
	};
	
	/**
	 * Create a new Schedule table under the given context.
	 * @param context The runtime context.
	 */
	public Schedule(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
	}
	
	/** 
	 * @return The minimum clock value (TIME attribute) over all scheduled tuples.
	 */
	public Long min() {
		Long min = Long.MAX_VALUE;
		for (Tuple t : this.tuples)
		{
			Long time = (Long) t.value(Field.TIME.ordinal());
			if (min > time)
				min = time;
		}
		return min;
	}
	
}
