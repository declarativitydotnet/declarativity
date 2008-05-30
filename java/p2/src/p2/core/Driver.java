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
		
		private static class TupleSetContainer {
			public Long time;
			public TupleSet insertions;
			public TupleSet deletions;
			public TupleSetContainer(String name, Long time) {
				this.time = time;
				insertions = new TupleSet(name);
				deletions = new TupleSet(name);
			}
		}
		
		public enum Field{PROGRAM, TIME, TUPLENAME, INSERTIONS, DELETIONS};
		public static final Class[] SCHEMA =  {
			String.class,   // Program name
			Long.class,     // Evaluation time
			String.class,   // Tuple name
			TupleSet.class, // Insertion tuple set
			TupleSet.class  // Deletions tuple set
		};

		public Evaluate() {
			super("evaluate", new TypeList(SCHEMA));
		}
		
		public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
			Hashtable<String, Hashtable<String, TupleSetContainer>> evaluations = 
				new Hashtable<String, Hashtable<String, TupleSetContainer>>();
			
			for (Tuple tuple : tuples) {
				String program = (String) tuple.value(Field.PROGRAM.ordinal());
				Long   time    = (Long) tuple.value(Field.TIME.ordinal());
				String tupleName = (String) tuple.value(Field.TUPLENAME.ordinal());
				
				if (!evaluations.containsKey(program)) {
					evaluations.put(program, new Hashtable<String, TupleSetContainer>());
				}
				
				if (!evaluations.get(program).containsKey(tupleName)) {
					evaluations.get(program).put(tupleName, new TupleSetContainer(tupleName, time));
				}
				TupleSet insertions  = (TupleSet) tuple.value(Field.INSERTIONS.ordinal());
				TupleSet deletions   = (TupleSet) tuple.value(Field.DELETIONS.ordinal());
				evaluations.get(program).get(tupleName).insertions.addAll(insertions);
				
				if (deletions != null) {
					evaluations.get(program).get(tupleName).deletions.addAll(deletions);
				}
			}
			
			TupleSet delta = new TupleSet(tuples.name());
			for (String program : evaluations.keySet()) {
				for (String tupleName : evaluations.get(program).keySet()) {
					delta.addAll(
							evaluate(System.program(program), tuples.name(), 
									 evaluations.get(program).get(tupleName).time,
									 evaluations.get(program).get(tupleName).insertions, 
									 evaluations.get(program).get(tupleName).deletions)); 
				}
			}
			return delta;
		}
		
		private TupleSet evaluate(Program program, String name, Long time, TupleSet insertions, TupleSet deletions) {
			Hashtable<String, Set<Query>> queries  = program.queries();
			Hashtable<String, Tuple> continuations = new Hashtable<String, Tuple>();
			java.lang.System.err.println("EVALUATE PROGRAM " + program.name() + " INSERTIONS " + insertions + " DELETIONS " + deletions);

			Table table = Table.table(insertions.name());
			try {
				if (table instanceof Aggregation && deletions.size() > 0) {
					deletions.remove(insertions);
					if (deletions.size() > 0) {
						TupleSet updates = table.delete(deletions);
						insertions.addAll(updates);
						deletions.clear();
					}
				}
				
				if (insertions.size() > 0) {
					if (!table.name().equals("periodic") || program.name().equals("runtime"))
						insertions = table.insert(insertions, deletions);
				}
			} catch (Exception e) {
				e.printStackTrace();
				java.lang.System.exit(0);
			}

			try {
				while (insertions.size() > 0) {
					java.lang.System.err.println("\tINSERTIONS " + insertions);

					if (insertions.size() == 0) continue;

					Set<Query> querySet = queries.get(insertions.name());
					if (querySet == null) break;

					TupleSet delta = new TupleSet(insertions.name());
					for (Query query : querySet) {
						if (query.event() != Table.Event.DELETE) {
							TupleSet result = query.evaluate(insertions);
							java.lang.System.err.println("\tRUN QUERY " + query.rule() + " input " + insertions);
							java.lang.System.err.println("\tQUERY " + query.rule() + " result " + result);

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
									continuation(continuations, program.name(), time, Table.Event.DELETE, result);
								}
								else {
									continuation(continuations, program.name(), time, Table.Event.INSERT, result);
								}
							}
						}
					}

					if (delta.size() > 0) {
						insertions = table.insert(delta, deletions);
					}
					else {
						insertions.clear();
					}
				}
			}
			catch (Exception e) {
				e.printStackTrace();
				java.lang.System.exit(1);
			}


			try {
				while (deletions.size() > 0) {
					if (table.type() == Table.Type.TABLE) {
						java.lang.System.err.println("\tDELETING TUPLES " + deletions);
						deletions = table.delete(deletions);
						java.lang.System.err.println("\tTABLE POST DELETION " + table.tuples());
					}
					else {
						java.lang.System.err.println("Can't delete tuples from non table type");
						java.lang.System.exit(0);
					}

					if (deletions.size() == 0) continue;

					TupleSet delta = new TupleSet(deletions.name());
					Set<Query> querySet = queries.get(delta.name());
					if (querySet == null) break;

					for (Query query : querySet) {
						if (query.event() != Table.Event.INSERT) {
							TupleSet result = query.evaluate(deletions);
							if (result.size() == 0) { 
								continue;
							}
							else if (result.name().equals(deletions.name())) {
								delta.addAll(result);
							}
							else {
								Table t = Table.table(result.name());
								if (t.type() == Table.Type.TABLE) {
									continuation(continuations, program.name(), time, Table.Event.DELETE, result);
								}
							}
						}
					}
					deletions = delta;
				}
			} catch (Exception e) {
				e.printStackTrace();
				java.lang.System.exit(1);
			}

			TupleSet delta = new TupleSet(name);
			for (Tuple continuation : continuations.values()) {
				continuation.name(name);
				delta.add(continuation);
			}

			java.lang.System.err.println("EVALUATION RESULT: " + delta);

			return delta;
		}

		private void continuation(Hashtable<String, Tuple> continuations, String program, Long time, Table.Event event, TupleSet result) {
			String key = program + "." + result.name();

			if (!continuations.containsKey(key)) {
				Tuple tuple = new Tuple(name(), program, time, result.name(), 
						new TupleSet(result.name()), new TupleSet(result.name()));
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
		/* Schedule all runtime program facts. */
		try {
			TupleSet tuples = Program.fact.secondary().get(
					new Key(FactTable.Field.PROGRAM.ordinal())).lookup(this.runtime.name());

			Hashtable<String, TupleSet> facts = new Hashtable<String, TupleSet>();
			if (tuples != null) {
				for (Tuple tuple : tuples) {
					Tuple fact = (Tuple) tuple.value(FactTable.Field.TUPLE.ordinal());
					if (!facts.containsKey(fact.name())) {
						facts.put(fact.name(), new TupleSet(fact.name()));
					}
					facts.get(fact.name()).add(fact);
				}
			}

			for (TupleSet fact : facts.values()) {
				try {
					java.lang.System.err.println("EVALUATE RUNTIME FACT " + fact);
					evaluate(fact, 0L);
				} catch (UpdateException e) {
					e.printStackTrace();
					java.lang.System.exit(1);
				}
			}
		} catch (BadKeyException e) {
			e.toString();
			return;
		}
		
		
		while (true) {
			try {
				Thread.sleep(5);
			} catch (InterruptedException e) { }
			
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
		}
	}
	
	public void evaluate(TupleSet tuples, Long clock) throws UpdateException {
		TupleSet evaluation = new TupleSet(System.evaluator().name());
		evaluation.add(new Tuple(System.evaluator().name(), runtime.name(), clock, 
								 tuples.name(), tuples, new TupleSet(tuples.name())));
		/* Evaluate until nothing left in this clock. */
		while (evaluation.size() > 0) {
			evaluation = System.evaluator().insert(evaluation, null);
		}
	}


}
