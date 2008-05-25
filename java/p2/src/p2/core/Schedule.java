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

public class Schedule extends ObjectTable {
	
	public static final Key PRIMARY_KEY = new Key(0,1,2);
	
	public enum Field {TIME, PROGRAM, TUPLENAME, INSERTIONS, DELETIONS};
	public static final Class[] SCHEMA = { 
		Long.class,     // Time
		String.class,   // Program name
		String.class,   // Tuple name
		TupleSet.class, // Insertion tuple set
		TupleSet.class  // Deletion tuple set
	};
	
	private Long min;

	public Schedule() {
		super("schedule", PRIMARY_KEY, new TypeList(SCHEMA));
		this.min = new Long(0);
	}
	
	public Long min() {
		return this.min;
	}
	
	@Override
	public TupleSet delete(TupleSet tuples) throws UpdateException {
		TupleSet delta = super.delete(tuples);
		this.min = 0L;
		for (Tuple tuple : tuples()) {
			min = min.longValue() < ((Long)tuple.value(Field.TIME.ordinal())).longValue() ?
					min : (Long) tuple.value(Field.TIME.ordinal());
		}
		return delta;
	}
	
	@Override
	protected boolean insert(Tuple tuple) throws UpdateException {
		min = min.longValue() < ((Long)tuple.value(Field.TIME.ordinal())).longValue() ?
				min : (Long) tuple.value(Field.TIME.ordinal());
		return super.insert(tuple);
	}
	
}
