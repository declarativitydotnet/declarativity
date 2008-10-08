package p2.core;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;
import p2.exec.Query;
import p2.lang.plan.Program;
import p2.lang.plan.Watch.WatchTable;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.operator.Operator;
import p2.types.operator.Watch;
import p2.types.table.Aggregation;
import p2.types.table.Table;
import p2.types.table.TableName;
import p2.lang.Compiler;

public class Driver implements Runnable {
	
	public static class Flusher extends p2.types.table.Function {
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
		
		public TupleSet aggregate(TupleSet tuples) {
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
		
		public enum Field{TIME, PROGRAM, TABLENAME, INSERTIONS, DELETIONS};
		public static final Class[] SCHEMA =  {
			Long.class,       // Time
			String.class,     // Program name
			TableName.class,  // Table name
			TupleSet.class,   // Insertion tuple set
			TupleSet.class    // Deletions tuple set
		};
		
		private Runtime context; 

		public Flusher(Runtime context) {
			super("flusher", new TypeList(SCHEMA));
			this.context = context;
		}
		
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
	
	public static class Evaluator extends p2.types.table.Function {
		public enum Field{TIME, PROGRAM, TABLENAME, INSERTIONS, DELETIONS};
		public static final Class[] SCHEMA =  {
			Long.class,       // Evaluation time
			String.class,     // Program name
			TableName.class,  // Table name
			TupleSet.class,   // Insertion tuple set
			TupleSet.class    // Deletions tuple set
		};
		
		private Runtime context;

		public Evaluator(Runtime context) {
			super("evaluator", new TypeList(SCHEMA));
			this.context = context;
		}
		
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
				continuation(continuations, time, program.name(), Table.Event.DELETE, deletions);

				for (Query query : querySet) {
					if (query.event() != Table.Event.DELETE) {
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
							continuation(continuations, time, program.name(), Table.Event.DELETE, result);
						}
						else {
							continuation(continuations, time, program.name(), Table.Event.INSERT, result);
						}
					}
				}
			}
			else if (deletions.size() > 0) {
				for (Query query : querySet) {
					Table output = context.catalog().table(query.output().name());
					if (query.event() == Table.Event.DELETE ||
							(output.type() == Table.Type.TABLE && query.event() != Table.Event.INSERT)) {
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
							continuation(continuations, time, program.name(), Table.Event.INSERT, result);
						}
						else if (output.type() == Table.Type.TABLE) {
							continuation(continuations, time, program.name(), Table.Event.DELETE, result);
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

		private void continuation(Hashtable<String, Tuple> continuations, Long time,
				                  String program, Table.Event event, TupleSet result) {
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

	public interface Task {
		public TupleSet insertions();
		
		public TupleSet deletions();
		
		public String program();

		public TableName name();
	}
	
	/* Tasks that the driver needs to execute during the next clock. */
	private List<Task> tasks;
	
	/** The schedule queue. */
	private Program runtime;

	private Schedule schedule;

	private Clock clock;
	
	public Evaluator evaluator;
	
	private Flusher flusher;

	public Driver(Runtime context, Schedule schedule, Clock clock) {
		this.tasks = new ArrayList<Task>();
		this.schedule = schedule;
		this.clock = clock;
		this.evaluator = new Evaluator(context);
		this.flusher = new Flusher(context);
		
		context.catalog().register(this.evaluator);
		context.catalog().register(this.flusher);
	}
	
	public void runtime(Program runtime) {
		this.runtime = runtime;
	}

	public void task(Task task) {
		this.tasks.add(task);
	}

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
	
	public void evaluate(String program, TableName name, TupleSet insertions, TupleSet deletions) throws UpdateException {
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
