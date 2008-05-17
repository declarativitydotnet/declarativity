package p2.core;

import java.util.HashSet;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Set;
import p2.exec.Query;
import p2.lang.plan.Predicate;
import p2.lang.plan.Program;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

public class Driver implements Runnable {
	
	public static class DriverTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1);

		public enum Field{PROGRAM, TUPLENAME, TUPLESET};
		public static final Class[] SCHEMA =  {
			String.class,   // Program name
			String.class,   // Tuple name
			TupleSet.class  // TupleSet to evaluate
		};

		public DriverTable() {
			super("driver", PRIMARY_KEY, new TypeList(SCHEMA));
		}

		protected boolean insert(Tuple tuple) throws UpdateException {
			String   name   = (String) tuple.value(Field.PROGRAM.ordinal());
			TupleSet tuples = (TupleSet) tuple.value(Field.TUPLESET.ordinal());
			p2.core.System.driver().evaluate(System.program(name), tuples);
			return true;  // Don't store the tuple
		}
	}

	/** The schedule queue. */
	private Program runtime;
	
	private Schedule schedule;
	
	private Clock clock;
	
	public Driver(Program runtime, Schedule schedule, Clock clock) {
		this.runtime = runtime;
		this.schedule = schedule;
		this.clock = clock;
	}
	

	public void run() {
		final TupleSet scheduleInsertions = new TupleSet(schedule.name());
		schedule.register(new Table.Callback() {
			public void deletion(TupleSet tuples) { /* Don't care. */ }
			public void insertion(TupleSet tuples) {
				synchronized (schedule) {
					scheduleInsertions.addAll(tuples);
					schedule.notify();
				}
			}
		});
		
		TupleSet factSchedule = new TupleSet(schedule.name());
		for (TupleSet fact : this.runtime.facts().values()) {
			Table table = Table.table(fact.name());
			try {
				if (!table.isEvent()) {
					TupleSet delta = table.insert(fact);
					if (delta.size() > 0) {
						factSchedule.add(
							new Tuple(schedule.name(), clock.current(), runtime.name(), 
									  delta.name(), Predicate.EventModifier.INSERT.toString(), delta));
					}
				}
				else {
					factSchedule.add(
						new Tuple(schedule.name(), clock.current(), runtime.name(), 
								  fact.name(), Predicate.EventModifier.NONE.toString(), fact));
				}
			} catch (UpdateException e) {
				e.printStackTrace();
				java.lang.System.exit(0);
			}
		}
		
		if (factSchedule.size() > 0) {
			try {
				schedule.insert(factSchedule);
			} catch (UpdateException e) {
				e.printStackTrace();
				java.lang.System.exit(1);
			}
		}
		
		Hashtable<String, TupleSet> insertions = new Hashtable<String, TupleSet>();
		while (true) {
			synchronized (schedule) {
				if (schedule.size() == 0) {
					try {
						java.lang.System.err.println("Nothing scheduled at this time.");
						schedule.wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
						java.lang.System.exit(1);
					}
				}
			}
			
			try {
				assert(this.clock.current() < schedule.min());
				
				// java.lang.System.err.println("CURRENT CLOCK " + clock.current() + " MIN SCHEDULED TIME " + schedule.min());
				/* Evaluate queries that run off the start clock. */
				if (this.clock.current() < schedule.min()) {
					TupleSet clock = this.clock.set(schedule.min());
					evaluate(this.runtime, clock); // Eval new clock
				}
				
				/* Schedule until nothing left in this clock. */
				while (scheduleInsertions.size() > 0) {
					TupleSet scheduled = new TupleSet(schedule.name());
					scheduled.addAll(scheduleInsertions);
					scheduleInsertions.clear();
					evaluate(this.runtime, scheduled);
				}
			} catch (UpdateException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}

	/** 
	 * Run fixpoint evaluation for a single strata. 
	 */
	private void evaluate(Program program, TupleSet insertion) {
		Hashtable<String, Set<Query>> queries = program.queries();
		Hashtable<String, Table>      tables  = program.tables();
		Hashtable<String, TupleSet> deletions = new Hashtable<String, TupleSet>();
		
		try {
			evaluate(program, insertion, deletions);
		} catch (P2RuntimeException e) {
			e.printStackTrace();
			java.lang.System.exit(1);
		}

		while (deletions.size() > 0) {
			for (String name : deletions.keySet()) {
				TupleSet runnable = deletions.remove(name);
				if (!queries.containsKey(runnable.name())) {
					// TODO log unknown tuple set
					java.lang.System.err.println("Unknown tuple set " + runnable.name());
					continue;
				}

				Set<Query> querySet = queries.get(runnable.name());
				for (Query query : querySet) {
					try {
						TupleSet result = query.evaluate(runnable);
						TupleSet delta = tables.get(result.name()).remove(result);
						if (tables.containsKey(delta.name()))
								update(deletions, delta);
						// TODO Consider schedule
					} catch (UpdateException e) {
						e.printStackTrace();
						java.lang.System.exit(0);
					} catch (P2RuntimeException e) {
						e.printStackTrace();
						java.lang.System.exit(0);
					}
				}
			}
		}
	}
	
	/**
	 * Fixed point evaluation of all events in the event set.
	 * @throws P2RuntimeException 
	 */
	private void evaluate(Program program, 
			              TupleSet eventSet, 
			              Hashtable<String, TupleSet> deletions) throws P2RuntimeException {
		Hashtable<String, Set<Query>> queries = program.queries(); 
		Hashtable<String, Table>      tables  = program.tables();
		TupleSet                      delta   = new TupleSet(eventSet.name());
		
		java.lang.System.err.println("EVALUATING PROGRAM " + program.name() + " on tupleset " + eventSet.name());
		
		for ( ; !eventSet.isEmpty(); eventSet = delta) {
			if (!queries.containsKey(eventSet.name())) {
				// TODO log unknown tuple set
				java.lang.System.err.println("Unknown tuple set " + eventSet.name() + 
						                     " in program " + program);
				continue;
			}

			Set<Query> querySet = queries.get(eventSet.name());
			for (Query query : querySet) {
				TupleSet result = query.evaluate(eventSet);
				if (tables.containsKey(result.name())) {
					if (query.delete()) {
						update(deletions, result);
					}
					else {
						try {
							TupleSet conflicts = tables.get(result.name()).conflict(result);
							result = tables.get(result.name()).insert(result);
							if (delta.name().equals(eventSet.name())) {
								delta.addAll(result);
							}
							else {
								schedule.add(clock.current(), program.name(), 
									         Predicate.EventModifier.INSERT.toString(), result);
							}
							update(deletions, conflicts);
						} catch (UpdateException e) {
							e.printStackTrace();
							java.lang.System.exit(0);
						}
					}
				}
				else {
					/* Fixpoint evaluation on event */
					evaluate(program, result, deletions);
				}
			}
		}
		
	}

	private void update(Hashtable<String, TupleSet> buffer, TupleSet result) {
		if (buffer.containsKey(result.name())) {
			buffer.get(result.name()).addAll(result);
		}
		else {
			buffer.put(result.name(), result);
		}
	}
}
