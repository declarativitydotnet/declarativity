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
import p2.types.exception.RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

public class Driver {
	

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

	/** The system logical clock. */
	private static Clock clock = new Clock("localhost");

	/** The schedule queue. */
	private Program runtime;
	
	private Schedule scheduleTable;
	
	public Driver(Program runtime, Schedule schedule) {
		this.runtime = runtime;
		this.scheduleTable = schedule;
	}
	

	public void run() {
		// TODO Schedule all facts for time 0.
		
		final TupleSet scheduleInsertions = new TupleSet(scheduleTable.name());
		scheduleTable.register(new Table.Callback() {
			public void deletion(TupleSet tuples) { /* Don't care. */ }
			public void insertion(TupleSet tuples) {
				scheduleInsertions.addAll(tuples);
			}
		});
		
		Hashtable<String, TupleSet> insertions = new Hashtable<String, TupleSet>();
		while (true) {
			if (scheduleTable.size() == 0) {
				// TODO Register callback, go to sleep
			}
			try {
				/* Evaluate queries that run off the start clock. */
				TupleSet clock = this.clock.set(scheduleTable.min());
				evaluate(this.runtime, clock); // Eval new clock
				
				/* Schedule until nothing left in this clock. */
				while (scheduleInsertions.size() > 0) {
					TupleSet scheduled = new TupleSet(scheduleTable.name());
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
		} catch (RuntimeException e) {
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
					} catch (RuntimeException e) {
						e.printStackTrace();
						java.lang.System.exit(0);
					}
				}
			}
		}
	}
	
	/**
	 * Fixed point evaluation of all events in the event queue.
	 * @param queries All queries to run events against.
	 * @param tables All tables that queries produce results on.
	 * @param eventQ The event queue.
	 * @param insertions The insertions that occur during FP.
	 * @param deletions The deletions that occur (will not be committed).
	 * @throws RuntimeException 
	 */
	private void evaluate(Program program, 
			              TupleSet eventSet, 
			              Hashtable<String, TupleSet> deletions) throws RuntimeException {
		Hashtable<String, Set<Query>> queries = program.queries(); 
		Hashtable<String, Table>      tables  = program.tables();
		TupleSet                      delta   = new TupleSet(eventSet.name());
		
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
								scheduleTable.schedule(clock.current(), program.name(), 
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
