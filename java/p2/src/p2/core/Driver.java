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
import p2.lang.plan.Fact.FactTable;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.Aggregation;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;
import p2.types.table.TableName;

public class Driver implements Runnable {
	
	public static class Evaluate extends p2.types.table.Function {
		private static class UpdateState {
			public Hashtable<String, TupleSet> insertions;
			public Hashtable<String, TupleSet> deletions;
			public UpdateState() {
				this.insertions = new Hashtable<String, TupleSet>();
				this.deletions  = new Hashtable<String, TupleSet>();
			}
		}
		
		private static class EvalState {
			public Long time;
			public String program;
			public TableName name;
			public TupleSet insertions;
			public TupleSet deletions;
			public EvalState(Long time, String program, TableName name) {
				this.time    = time;
				this.program = program;
				this.name    = name;
				insertions = new TupleSet(name);
				deletions = new TupleSet(name);
			}
			@Override
			public int hashCode() {
				return toString().hashCode();
			}
			@Override
			public boolean equals(Object o) {
				return o instanceof EvalState &&
				       toString().equals(o.toString());
			}
			@Override
			public String toString() {
				return this.program + ":" + time.toString() + ":" +  name;
			}
		}
		
		public enum Field{TIME, PROGRAM, TABLENAME, INSERTIONS, DELETIONS};
		public static final Class[] SCHEMA =  {
			Long.class,       // Evaluation time
			String.class,     // Program name
			TableName.class,  // Table name
			TupleSet.class,   // Insertion tuple set
			TupleSet.class    // Deletions tuple set
		};

		public Evaluate() {
			super("evaluate", new TypeList(SCHEMA));
		}
		
		public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
			Hashtable<EvalState, EvalState> evaluations = new Hashtable<EvalState, EvalState>();
			
			for (Tuple tuple : tuples) {
				Long   time    = (Long) tuple.value(Field.TIME.ordinal());
				String program = (String) tuple.value(Field.PROGRAM.ordinal());
				TableName name = (TableName) tuple.value(Field.TABLENAME.ordinal());
				EvalState state = new EvalState(time, program, name);
				
				if (!evaluations.containsKey(state)) {
					evaluations.put(state, state);
				}
				else {
					state = evaluations.get(state);
				}
				
				TupleSet insertions  = (TupleSet) tuple.value(Field.INSERTIONS.ordinal());
				TupleSet deletions   = (TupleSet) tuple.value(Field.DELETIONS.ordinal());
				
				if (insertions != null) {
					state.insertions.addAll(insertions);
				}
				
				if (deletions != null) {
					state.deletions.addAll(deletions);
				}
			}
			
			TupleSet delta = new TupleSet(name());
			for (EvalState state : evaluations.values()) {
				delta.addAll(evaluate(state.time, System.program(state.program),
								      state.name, state.insertions, state.deletions));
			}
			return delta;
		}
		
		private TupleSet evaluate(Long time, Program program, TableName name, TupleSet insertions, TupleSet deletions) {
			Hashtable<String, Tuple> continuations = new Hashtable<String, Tuple>();
			java.lang.System.err.println("EVALUATE PROGRAM " + program.name() + " " + name + ": INSERTIONS " + insertions + " DELETIONS " + deletions);

			Table table = Table.table(name);
			try {
				do { 
					insertions = table.insert(insertions, deletions);
					java.lang.System.err.println("\tINSERTION DELTAS " + name + ": " + insertions);
					if (insertions.size() == 0) break;

					Set<Query> querySet = program.queries(insertions.name());
					if (querySet == null) break;

					TupleSet delta = new TupleSet(insertions.name());
					for (Query query : querySet) {
						if (query.event() != Table.Event.DELETE) {
							TupleSet result = query.evaluate(insertions);
							java.lang.System.err.println("\t\tRUN QUERY " + query.rule() + " input " + insertions);
							java.lang.System.err.println("\t\tQUERY " + query.rule() + " result " + result);

							if (result.size() == 0) { 
								continue;
							}
							else if (result.name().equals(insertions.name())) {
								if (query.delete()) {
									deletions.addAll(result);
								}
								else {
									delta.addAll(result);
								}
							}
							else {
								if (query.delete()) {
									continuation(continuations, time, program.name(), Table.Event.DELETE, result);
								}
								else {
									continuation(continuations, time, program.name(), Table.Event.INSERT, result);
								}
							}
						}
					}
					insertions = delta;
				} while (insertions.size() > 0);
			}
			catch (Exception e) {
				e.printStackTrace();
				java.lang.System.exit(1);
			}


			try {
				if (!(table instanceof Aggregation) && deletions.size() > 0) {
					if (table.type() == Table.Type.TABLE) {
						java.lang.System.err.println("\tDELETING TUPLES " + name + ": " + deletions);
						deletions = table.delete(deletions);
						java.lang.System.err.println("\t\tTABLE POST DELETION " + table.tuples());
					}
					else {
						java.lang.System.err.println("Can't delete tuples from non table type");
						java.lang.System.exit(0);
					}

					TupleSet delta = new TupleSet(deletions.name());
					Set<Query> queries = program.queries(delta.name());

					if (queries != null) {
						for (Query query : queries) {
							if (query.event() != Table.Event.INSERT) {
								TupleSet result = query.evaluate(deletions);
								if (result.size() == 0) { 
									continue;
								}
								else if (!result.name().equals(deletions.name())) {
									Table t = Table.table(result.name());
									if (t.type() == Table.Type.TABLE) {
										continuation(continuations, time, program.name(), Table.Event.DELETE, result);
									}
								}
							}
						}
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
				java.lang.System.exit(1);
			}

			TupleSet delta = new TupleSet(name);
			for (Tuple continuation : continuations.values()) {
				delta.add(continuation);
			}

			java.lang.System.err.println("==================== RESULT " + name + ": " + delta + "\n\n");

			return delta;
		}

		private void continuation(Hashtable<String, Tuple> continuations, Long time, String program, Table.Event event, TupleSet result) {
			String key = program + "." + result.name();

			if (!continuations.containsKey(key)) {
				Tuple tuple = new Tuple(time, program, result.name(),
						                new TupleSet(result.name()), 
						                new TupleSet(result.name()));
				continuations.put(key, tuple);
			}

			if (event == Table.Event.INSERT) {
				TupleSet insertions = (TupleSet) continuations.get(key).value(Field.INSERTIONS.ordinal());
				insertions.addAll(result);
			}
			else {
				TupleSet deletions = (TupleSet) continuations.get(key).value(Field.DELETIONS.ordinal());
				deletions.addAll(result);
			}
		}
	}

	/** The schedule queue. */
	private Program runtime;

	private Schedule schedule;

	private Periodic periodic;

	private Clock clock;

	public Driver(Program runtime, Schedule schedule, Periodic periodic, Clock clock) {
		this.runtime = runtime;
		this.schedule = schedule;
		this.periodic = periodic;
		this.clock = clock;
	}


	public void run() {
		while (true) {
			synchronized (this) {
				Long min = Long.MAX_VALUE;
				
				if (schedule.cardinality() > 0) {
					min = min < schedule.min() ? min : schedule.min();
				}
				
				if (min == Long.MAX_VALUE) {
					continue;
				}
				else {
					try {
						java.lang.System.err.println("============================ EVALUATE =============================");
						evaluate(clock.time(min), min);
						java.lang.System.err.println("============================ ======== =============================");
					} catch (UpdateException e) {
						e.printStackTrace();
					}
				}
			}
			
			try {
				Thread.sleep(5);
			} catch (InterruptedException e) { }
			
		}
	}
	
	public void evaluate(TupleSet tuples, Long clock) throws UpdateException {
		TupleSet evaluation = new TupleSet(System.evaluator().name());
		evaluation.add(new Tuple(clock, runtime.name(), tuples.name(), tuples, 
								 new TupleSet(tuples.name())));
		/* Evaluate until nothing left in this clock. */
		while (evaluation.size() > 0) {
			evaluation = System.evaluator().insert(evaluation, null);
		}
	}


}
