package jol.core;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;
import jol.types.table.Function;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;

/**
 * The periodic table stores all registered periodics.
 * Programs can remove periodics by simply deleting from
 * this table. The compiler will translate all periodic
 * statements within the program into a tuple entry that
 * is registered in this table. The runtime schedules
 * periodics from the tuple entries in this table.
 *
 */
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
				String program    = (String) tuple.value(Periodic.Field.PROGRAM.ordinal());
				Long   time       = (Long) tuple.value(Periodic.Field.TIME.ordinal());
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
	
	public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "periodic");
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
	
	public Periodic(Runtime context, Table schedule) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		context.catalog().register(new Scheduler(schedule));
	}
	
	@Override
	public boolean insert(Tuple tuple) throws UpdateException {
		String identifier = (String) tuple.value(Field.IDENTIFIER.ordinal());
		if (identifier == null) {
			tuple.value(Field.IDENTIFIER.ordinal(), Runtime.idgen().toString());
		}
		
		return super.insert(tuple);
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
