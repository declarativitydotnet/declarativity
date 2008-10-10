package jol.core;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;
import jol.exec.Query;
import jol.lang.plan.Predicate;
import jol.lang.plan.Program;
import jol.lang.plan.Watch.WatchTable;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.P2RuntimeException;
import jol.types.exception.UpdateException;
import jol.types.operator.Operator;
import jol.types.operator.Watch;
import jol.types.table.Aggregation;
import jol.types.table.Table;
import jol.types.table.TableName;

/**
 * The main driver loop that executes query objects {@link Query}.
 */
public class Driver implements Runnable {
	
	/**
	 * A table function that flushes {@link TupleSet} objects to
	 * the respective tables. The flush operation generates a new
	 * {@link TupleSet} object containing the delta tuple set, which
	 * should be executed by the query objects {@link Query}.
	 *
	 */
	public static class Flusher extends jol.types.table.Function {
		private static class ScheduleUnit {
			public Long time;
			public String program;
			public TableName name;
			public TupleSet insertions;
			public TupleSet deletions;
			
			public ScheduleUnit(Tuple tuple) {
				this.time    = (Long)      tuple.value(Field.TIME.ordinal());
				this.program = (String)    tuple.value(Field.PROGRAM.ordinal());
				this.name    = (TableName) tuple.value(Field.TABLENAME.ordinal());
				insertions = new TupleSet(name);
				deletions = new TupleSet(name);
			}
			
			public void add(Tuple tuple) {
				TupleSet  insertions = (TupleSet)  tuple.value(Field.INSERTIONS.ordinal());
				TupleSet  deletions  = (TupleSet)  tuple.value(Field.DELETIONS.ordinal());
				if (insertions != null) this.insertions.addAll(insertions);
				if (deletions != null) this.deletions.addAll(deletions);
			}
			
			public Tuple tuple() {
				return new Tuple(this.time, this.program, this.name, this.insertions, this.deletions);
			}
			
			@Override
			public int hashCode() {
				return toString().hashCode();
			}
			@Override
			public boolean equals(Object o) {
				return o instanceof ScheduleUnit &&
				       toString().equals(o.toString());
			}
			@Override
			public String toString() {
				return this.program + ":" + ":" + time.toString() + ":" +  name;
			}
		}
		
		/**
		 * Aggregates the set of tuples into groups
		 * of tuples that are flushed into the same table.
		 * @param tuples The set of tuples to be flushed
		 * @return Aggregated set of tuples.
		 */
		private TupleSet aggregate(TupleSet tuples) {
			Hashtable<ScheduleUnit, ScheduleUnit> units = new Hashtable<ScheduleUnit, ScheduleUnit>();
			for (Tuple tuple : tuples) {
				ScheduleUnit unit = new ScheduleUnit(tuple);
				if (!units.containsKey(unit)) {
					units.put(unit, unit);
				}
				units.get(unit).add(tuple);
			}
			TupleSet aggregate = new TupleSet(name());
			for (ScheduleUnit unit : units.keySet()) {
				if (unit.insertions.size() > 0 || unit.deletions.size() > 0)
					aggregate.add(unit.tuple());
			}
			return aggregate;
		}
		
		/** The attribute fields expected by this table function. */
		public enum Field{TIME, PROGRAM, TABLENAME, INSERTIONS, DELETIONS};
		
		/** The attribute types execpted by this table function. */
		public static final Class[] SCHEMA =  {
			Long.class,       // Time
			String.class,     // Program name
			TableName.class,  // Table name
			TupleSet.class,   // Insertion tuple set
			TupleSet.class    // Deletions tuple set
		};
		
		/** The runtime context. */
		private Runtime context; 

		/** 
		 * Creates a new flusher table function.
		 * @param context The runtime context. 
		 */
		public Flusher(Runtime context) {
			super("flusher", new TypeList(SCHEMA));
			this.context = context;
		}
		
		@Override
		public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
			TupleSet delta = new TupleSet(name());
			for (Tuple tuple : aggregate(tuples)) {
				Long      time       = (Long)      tuple.value(Field.TIME.ordinal());
				String    program    = (String)    tuple.value(Field.PROGRAM.ordinal());
				TableName name       = (TableName) tuple.value(Field.TABLENAME.ordinal());
				TupleSet  insertions = (TupleSet)  tuple.value(Field.INSERTIONS.ordinal());
				TupleSet  deletions  = (TupleSet)  tuple.value(Field.DELETIONS.ordinal());
				
				if (insertions == null) insertions = new TupleSet(name);
				if (deletions == null)  deletions = new TupleSet(name);
				
				if (insertions.size() == 0 && deletions.size() == 0) {
					continue;
				}
				
				WatchTable watch = (WatchTable) context.catalog().table(WatchTable.TABLENAME);
				Table table = context.catalog().table(name);
				if (insertions.size() > 0 || table instanceof Aggregation) {
					insertions = table.insert(insertions, deletions);
					
					if (table instanceof Aggregation) {
						Operator watchRemove = watch.watched(program, name, Watch.Modifier.ERASE);
						if (watchRemove != null) {
							try { watchRemove.evaluate(deletions);
							} catch (P2RuntimeException e) { }
						}
					}
				}
				else { 
					if (table.type() != Table.Type.TABLE) return new TupleSet(name);
					
					deletions = table.delete(deletions);
					
					Operator watchRemove = watch.watched(program, name, Watch.Modifier.ERASE);
					if (watchRemove != null) {
						try { watchRemove.evaluate(deletions);
						} catch (P2RuntimeException e) { }
					}
				}
				
				if (insertions.size() > 0) {
					Operator watchAdd = watch.watched(program, name, Watch.Modifier.ADD);
					if (watchAdd != null) {
						try { watchAdd.evaluate(insertions);
						} catch (P2RuntimeException e) { }
					}
				}
				
				tuple.value(Field.INSERTIONS.ordinal(), insertions);
				tuple.value(Field.DELETIONS.ordinal(), deletions);
				delta.add(tuple);
			}
			return delta;
		}
	}
	
	/**
	 * Table function represents the query processor that evaluates 
	 * tuples using the query objects install by the runtime programs.
	 * The tuple format that this function expects is as follows:
	 * <Time, Program, TableName, Insertions, Deletions>
	 * Time: The evaluation time (usually taken from the system clock).
	 * Program: The program whose query objects are to evaluate the tuples.
	 * TableName: The name of the table to which the tuples refer.
	 * Insertions: Contains a set of tuples that represent the delta set
	 * of an insertion into the respective table.
	 * Deletions: Contains a set of tuples representing the delta set
	 * of a deletion from the respective table.
	 * NOTE: Deletions are not evaluated unless there are no insertions
	 * The return value of this table function contains a set of tuples
	 * that represent the output of the query evaluations. 
	 */
	public static class Evaluator extends jol.types.table.Function {
		/** The fields expected by this table function. */
		public enum Field{TIME, PROGRAM, TABLENAME, INSERTIONS, DELETIONS};
		/** The field types expected by this table function. */
		public static final Class[] SCHEMA =  {
			Long.class,       // Evaluation time
			String.class,     // Program name
			TableName.class,  // Table name
			TupleSet.class,   // Insertion tuple set
			TupleSet.class    // Deletions tuple set
		};
		
		/** The runtime context. */
		private Runtime context;

		/**
		 * Creates a new evaluated based on the given runtime.
		 * @param context The runtime context. 
		 */
		public Evaluator(Runtime context) {
			super("evaluator", new TypeList(SCHEMA));
			this.context = context;
		}
		
		@Override
		public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
			TupleSet delta = new TupleSet(name());
			for (Tuple tuple : tuples) {
				Long      time       = (Long)      tuple.value(Field.TIME.ordinal());
				String    programName= (String)    tuple.value(Field.PROGRAM.ordinal());
				TableName name       = (TableName) tuple.value(Field.TABLENAME.ordinal());
				TupleSet  insertions = (TupleSet)  tuple.value(Field.INSERTIONS.ordinal());
				TupleSet  deletions  = (TupleSet)  tuple.value(Field.DELETIONS.ordinal());
				if (deletions == null) deletions = new TupleSet(name);
				Program program = context.program(programName);
				
				if (program != null) {
					TupleSet  result = evaluate(time, program, name, insertions, deletions);
					delta.addAll(result);
				}
				else {
					java.lang.System.err.println("EVALUATOR ERROR: unknown program " + programName + "!");
				}
			}
			return delta;
		}
		
		/** 
		 * Evaluates the given insertions/deletions against the program queries.
		 * @param time The current time.
		 * @param program The program whose queries will evaluate the tuples.
		 * @param name The name of the table to which the tuples refer.
		 * @param insertions A delta set of tuple insertions.
		 * @param deletions A delta set of tuple deletions.
		 * @return The result of the query evaluation. NOTE: if insertions.size() > 0 then
		 * deletions will not be evaluated but rather deferred until a call is made with
		 * insertions.size() == 0.
		 * @throws UpdateException On evaluation error.
		 */
		private TupleSet evaluate(Long time, Program program, TableName name, TupleSet insertions, TupleSet deletions) 
		throws UpdateException {
			Hashtable<String, Tuple> continuations = new Hashtable<String, Tuple>();

			WatchTable watch = (WatchTable) context.catalog().table(WatchTable.TABLENAME);
			Operator watchInsert = watch.watched(program.name(), name, Watch.Modifier.INSERT);
			Operator watchDelete = watch.watched(program.name(), name, Watch.Modifier.DELETE);

			Set<Query> querySet = program.queries(name);
			if (querySet == null) {
				return new TupleSet(name); // Done
			}
			
			if (insertions.size() > 0) {
				/* We're not going to deal with the deletions yet. */
				continuation(continuations, time, program.name(), Predicate.Event.DELETE, deletions);

				for (Query query : querySet) {
					if (query.event() != Predicate.Event.DELETE) {
						if (watchInsert != null) {
							try { watchInsert.rule(query.rule()); watchInsert.evaluate(insertions);
							} catch (P2RuntimeException e) { 
								java.lang.System.err.println("WATCH INSERTION FAILURE ON " + name + "!");
							}
						}
						
						TupleSet result = null;
						try {
							result = query.evaluate(insertions);
							if (result.size() == 0) continue;
						} catch (P2RuntimeException e) {
							e.printStackTrace();
							java.lang.System.exit(0);
						}

						if (query.isDelete()) {
							continuation(continuations, time, program.name(), Predicate.Event.DELETE, result);
						}
						else {
							continuation(continuations, time, program.name(), Predicate.Event.INSERT, result);
						}
					}
				}
			}
			else if (deletions.size() > 0) {
				for (Query query : querySet) {
					Table output = context.catalog().table(query.output().name());
					if (query.event() == Predicate.Event.DELETE ||
							(output.type() == Table.Type.TABLE && query.event() != Predicate.Event.INSERT)) {
						if (watchDelete != null) {
							try { watchDelete.rule(query.rule()); watchDelete.evaluate(deletions);
							} catch (P2RuntimeException e) { }
						}
						
						TupleSet result = null;
						try {
							result = query.evaluate(deletions);
							if (result.size() == 0) continue;
						} catch (P2RuntimeException e) {
							e.printStackTrace();
							java.lang.System.exit(0);
						}
						
						if (!query.isDelete() && output.type() == Table.Type.EVENT) {
							/* Query is not a delete and it's output type is an event. */
							continuation(continuations, time, program.name(), Predicate.Event.INSERT, result);
						}
						else if (output.type() == Table.Type.TABLE) {
							continuation(continuations, time, program.name(), Predicate.Event.DELETE, result);
						}
						else {
							throw new UpdateException("Query " + query + " is trying to delete from table " + output.name() + "?");
						}
					}
				}
			}

			TupleSet delta = new TupleSet(name);
			for (Tuple continuation : continuations.values()) {
				TupleSet ins  = (TupleSet) continuation.value(Field.INSERTIONS.ordinal());
				TupleSet dels = (TupleSet) continuation.value(Field.DELETIONS.ordinal());
				if (ins.size() > 0 || dels.size() > 0) {
					delta.add(continuation);
				}
			}

			return delta;
		}

		/**
		 * Helper routine that packages up result tuples grouped by (time, program, tablename).
		 * @param continuations Hashtable containing the tuple groups
		 * @param time Current time.
		 * @param program Program that evaluated the tuples.
		 * @param event Indicates whether the tuples are insertions or deletions.
		 * @param result The result tuples taken from the output of a query.
		 */
		private void continuation(Hashtable<String, Tuple> continuations, Long time,
				                  String program, Predicate.Event event, TupleSet result) {
			String key = program + "." + result.name();

			if (!continuations.containsKey(key)) {
				Tuple tuple = new Tuple(time, program, result.name(),
						                new TupleSet(result.name()), 
						                new TupleSet(result.name()));
				continuations.put(key, tuple);
			}

			if (event == Predicate.Event.INSERT) {
				TupleSet insertions = (TupleSet) continuations.get(key).value(Field.INSERTIONS.ordinal());
				insertions.addAll(result);
			}
			else {
				TupleSet deletions = (TupleSet) continuations.get(key).value(Field.DELETIONS.ordinal());
				deletions.addAll(result);
			}
		}
	}

	/**
	 * Tasks are used to inject tuples into the schedule.
	 */
	public interface Task {
		/** Insertion tuples */
		public TupleSet insertions();
		
		/** Deletion tuples. */
		public TupleSet deletions();
		
		/** The program name that should evaluate the tuples. */
		public String program();

		/** The name of the table to which the tuples belong. */
		public TableName name();
	}
	
	/** Tasks that the driver needs to execute during the next clock. */
	private List<Task> tasks;
	
	/** The runtime program */
	private Program runtime;

	/** The table containing all tuples that need to be scheduled/executed. */
	private Schedule schedule;

	/** The system clock. */
	private Clock clock;
	
	/** The evaluator table function. */
	public Evaluator evaluator;
	
	/** The flusher table function. */
	private Flusher flusher;

	/**
	 * Creates a new driver.
	 * @param context The runtime context.
	 * @param schedule The schedule table.
	 * @param clock The system clock table.
	 */
	public Driver(Runtime context, Schedule schedule, Clock clock) {
		this.tasks = new ArrayList<Task>();
		this.schedule = schedule;
		this.clock = clock;
		this.evaluator = new Evaluator(context);
		this.flusher = new Flusher(context);
		
		context.catalog().register(this.evaluator);
		context.catalog().register(this.flusher);
	}
	
	/**
	 * Set the runtime program.
	 * @param runtime The runtime program. 
	 */
	public void runtime(Program runtime) {
		this.runtime = runtime;
	}

	/**
	 * Add a task to the task queue. This will be evaluated
	 * on the next clock tick.
	 * @param task The task to be added.
	 */
	public void task(Task task) {
		this.tasks.add(task);
	}

	/**
	 * The main driver loop.
	 * The loop is responsible for 1. updating the clock 2. scheduling tasks.
	 * Driver algorithm:
	 * 	loop forever {
	 * 		update to new clock value;
	 * 		evaluate (insertion of clock value);
	 * 		evaluate (all tasks);
	 * 		evaluate (deletion of clock value);
	 * 	}
	 */
	public void run() {
		TupleSet time = clock.time(0L);
		while (true) {
			synchronized (this) {
				try {
					java.lang.System.err.println("============================     EVALUATE SCHEDULE     =============================");
					evaluate(runtime.name(), time.name(), time, null); // Clock insert current
					
					/* Evaluate task queue. */
					for (Task task : tasks) {
						evaluate(task.program(), task.name(), task.insertions(), task.deletions());
					}
					tasks.clear(); // Clear task queue.
					evaluate(runtime.name(), time.name(), null, time); // Clock delete current
					java.lang.System.err.println("============================ ========================== =============================");
				} catch (UpdateException e) {
					e.printStackTrace();
					java.lang.System.exit(1);
				}

				/* Check for new tasks or schedules, if none wait. */
				while (this.tasks.size() == 0 && schedule.cardinality() == 0) {
					try {
						this.wait();
					} catch (InterruptedException e) { }
				}
				if (schedule.cardinality() > 0) {
					time = clock.time(schedule.min());
				}
				else {
					time = clock.time(clock.current() + 1);
				}
			}
		}
	}
	
	/**
	 * Helper function that calls the flusher and evaluator table functions.
	 * This function will evaluate the passed in tuples to fixedpoint (until
	 * no further tuples exist to evaluate).
	 * @param program The program that should be executed.
	 * @param name The table name to which the tuples refer.
	 * @param insertions The set of insertions to evaluate.
	 * @param deletions The set of deletions to evaluate.
	 * @throws UpdateException
	 */
	private void evaluate(String program, TableName name, TupleSet insertions, TupleSet deletions) throws UpdateException {
		TupleSet insert = new TupleSet();
		TupleSet delete = new TupleSet();
		insert.add(new Tuple(clock.current(), program, name, insertions, deletions)); 
		/* Evaluate until nothing remains. */
		while (insert.size() > 0 || delete.size() > 0) {
			TupleSet delta = null;
			while(insert.size() > 0) {
				delta = flusher.insert(insert, null);
				delta = evaluator.insert(delta, null);
				insert.clear();
				split(delta, insert, delete);
			}
			
			while(delete.size() > 0) {
				delta = flusher.insert(delete, null);
				delta = evaluator.insert(delta, null);
				delete.clear();
				split(delta, insert, delete);
			}
		}
	}
	
	private void split(TupleSet tuples, TupleSet insertions, TupleSet deletions) {
		for (Tuple tuple : tuples) {
			Tuple insert = tuple.clone();
			Tuple delete = tuple.clone();
			insert.value(Evaluator.Field.INSERTIONS.ordinal(), tuple.value(Evaluator.Field.INSERTIONS.ordinal()));
			insert.value(Evaluator.Field.DELETIONS.ordinal(), null);
			delete.value(Evaluator.Field.INSERTIONS.ordinal(), null);
			delete.value(Evaluator.Field.DELETIONS.ordinal(), tuple.value(Evaluator.Field.DELETIONS.ordinal()));
			
			insertions.add(insert);
			deletions.add(delete);
		}
	}


}
