package p2.core;

import java.util.Hashtable;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.BasicTable;
import p2.types.table.Function;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;
import p2.types.table.TableName;

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
			
			TupleSet schedule = new TupleSet(this.schedule.name());
			TupleSet deltas   = new TupleSet(name());
			for (Tuple tuple : tuples) {
				String program = (String) tuple.value(Periodic.Field.PROGRAM.ordinal());
				Long   time    = (Long) tuple.value(Periodic.Field.TIME.ordinal());
				TupleSet periodics = new TupleSet(new TableName(program, "periodic"));
				periodics.add(tuple.clone());
				schedule.add(new Tuple(time, program, periodics.name(), periodics, null));
				deltas.add(tuple.clone());
			}
			
			if (schedule.size() > 0) {
				this.schedule.insert(schedule, conflicts);
			}
			
			return deltas;
		}
		
	}
	
	public static final Key PRIMARY_KEY = new Key(0);
	
	public enum Field {IDENTIFIER, PERIOD, TTL, TIME, COUNT, PROGRAM};
	public static final Class[] SCHEMA = { 
		String.class, // Identifier
		Long.class,   // Period
		Long.class,   // TTL
		Long.class,   // Time
		Long.class,   // Count
		String.class  // Program
	};
	
	private Scheduler scheduler;
	
	public Periodic(Table schedule) {
		super(new TableName(GLOBALSCOPE, "periodic"), PRIMARY_KEY, new TypeList(SCHEMA));
		this.scheduler = new Scheduler(schedule);
	}
	
	public Long min() {
		Long min = Long.MAX_VALUE;
		for (Tuple current : tuples) {
			Long time = (Long) current.value(Field.TIME.ordinal());
			min = min < time  ? min : time;
		}
		return min;
	}
}
