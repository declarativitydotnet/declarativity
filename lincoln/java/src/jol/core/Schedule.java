package jol.core;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class Schedule extends ObjectTable {
	
	public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "schedule");
	public static final Key PRIMARY_KEY = new Key();
	
	public enum Field {TIME, PROGRAM, TABLENAME, INSERTIONS, DELETIONS};
	public static final Class[] SCHEMA = { 
		Long.class,      // Time
		String.class,    // Program name
		TableName.class, // Table name
		TupleSet.class,  // Insertion tuple set
		TupleSet.class   // Deletion tuple set
	};
	
	public Schedule(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
	}
	
	public Long min() {
		Long min = Long.MAX_VALUE;
		for (Tuple tuple : tuples()) {
			min = min.longValue() < ((Long)tuple.value(Field.TIME.ordinal())).longValue() ?
						min : (Long) tuple.value(Field.TIME.ordinal());
		}
		return min;
	}
	
}
