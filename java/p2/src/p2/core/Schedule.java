package p2.core;

import java.util.ArrayList;
import java.util.List;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.operator.Operator;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;

public class Schedule extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key();
	
	public enum Field {TIME, PROGRAM, INSERTIONS, DELETIONS};
	public static final Class[] SCHEMA = { 
		Long.class,      // Time
		String.class,    // Program name
		TableName.class, // Table name
		TupleSet.class,  // Insertion tuple set
		TupleSet.class   // Deletion tuple set
	};
	
	public Schedule() {
		super(new TableName(GLOBALSCOPE, "schedule"), PRIMARY_KEY, new TypeList(SCHEMA));
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
