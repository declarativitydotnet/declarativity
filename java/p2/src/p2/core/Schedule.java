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
	
	public static final Key PRIMARY_KEY = new Key();
	
	public enum Field {TIME, PROGRAM, TUPLENAME, EVENT, SET};
	public static final Class[] SCHEMA = { 
		Long.class,     // Time
		String.class,   // Program name
		String.class,   // Tuple name
		String.class,   // Event description
		TupleSet.class  // Tuple set
	};
	
	private Long min;

	public Schedule() {
		super("schedule", PRIMARY_KEY, new TypeList(SCHEMA));
		this.min = new Long(0);
	}
	
	public Long min() {
		return this.min;
	}
	
	public void add(Long clock, String program, String event, TupleSet tuples) {
		try {
			insert(new Tuple(name(), clock, program, tuples.name(), event, tuples));
		} catch (UpdateException e) {
			e.printStackTrace();
		}
	}
	
	@Override
	protected boolean insert(Tuple tuple) throws UpdateException {
		TupleSet previous = primary().lookup(tuple);
		if (previous != null) {
			TupleSet newSet = (TupleSet) tuple.value(Field.SET.ordinal());
			for (Tuple prev : previous) {
				TupleSet prevSet = (TupleSet) tuple.value(Field.SET.ordinal());
				prevSet.addAll(newSet);
			}
		}
		min = min.longValue() < ((Long)tuple.value(Field.TIME.ordinal())).longValue() ?
				min : (Long) tuple.value(Field.TIME.ordinal());
		return super.insert(tuple);
	}
}
