package p2.core;

import java.util.Hashtable;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Function;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

public class Periodic extends ObjectTable {
	
	public static class Scheduler extends Function {
		
		private Table schedule;

		public Scheduler(Table schedule) {
			super("periodicScheduler", new TypeList(Periodic.SCHEMA));
			this.schedule = schedule;
		}

		@Override
		public TupleSet insert(TupleSet tuples, TupleSet conflicts)
				throws UpdateException {
			
			java.lang.System.err.println("INSERT INTO PERIODIC SCHEDULE " + tuples);
			TupleSet schedule = new TupleSet(this.schedule.name());
			TupleSet deltas   = new TupleSet(name());
			for (Tuple tuple : tuples) {
				String program = (String) tuple.value(Periodic.Field.PROGRAM.ordinal());
				Long   time    = (Long) tuple.value(Periodic.Field.TIME.ordinal());
				TupleSet periodic = new TupleSet(tuples.name());
				periodic.add(tuple);
				schedule.add(new Tuple(this.schedule.name(), time, program, periodic.name(), periodic, null));
				Tuple delta = tuple.clone();
				delta.name(name());
				deltas.add(delta);
			}
			
			if (schedule.size() > 0) {
				java.lang.System.err.println("SCHEDULE PERIODICS " + schedule);
				this.schedule.insert(schedule, conflicts);
			}
			
			return deltas;
		}
		
	}
	
	public static final Key PRIMARY_KEY = new Key(0);
	
	public enum Field {IDENTIFIER, PERIOD, TTL, COUNT, TIME, PROGRAM};
	public static final Class[] SCHEMA = { 
		String.class, // Identifier
		Long.class,   // Period
		Long.class,   // TTL
		Long.class,   // Count
		Long.class,   // Time
		String.class  // Program
	};
	
	private Scheduler scheduler;

	public Periodic(Table schedule) {
		super("periodic", PRIMARY_KEY, new TypeList(SCHEMA));
		this.scheduler = new Scheduler(schedule);
	}
	
	protected boolean insert(Tuple tuple) throws UpdateException {
		Long ttl   = (Long) tuple.value(Field.TTL.ordinal());
		Long count = (Long) tuple.value(Field.COUNT.ordinal());
		Long start = (Long) tuple.value(Field.TIME.ordinal());
		return super.insert(tuple);
	}
}
