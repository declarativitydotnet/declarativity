package p2.core;

import java.util.Hashtable;

import p2.core.Driver.Evaluate;
import p2.exec.Query.QueryTable;
import p2.lang.Compiler;
import p2.lang.Compiler.CompileTable;
import p2.lang.plan.Program;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.PlannerException;
import p2.types.exception.UpdateException;
import p2.types.table.Table;

public class System {
	private static String RUNTIME = "/Users/tcondie/workspace/P2/src/p2/core/runtime.olg";
			
	private static boolean initialized = false;
	
	private static Thread system;
	
	private static QueryTable query;
	
	private static Evaluate evaluator;
	
	private static Driver driver;
	
	private static Schedule schedule;
	
	/** The system logical clock. */
	private static Clock clock;
	
	private static Periodic periodic;
	
	private static Log log;
	
	private static Hashtable<String, Program> programs;
	
	public static Clock clock() {
		return clock;
	}
	
	public static Evaluate evaluator() {
		return evaluator;
	}
	
	public static QueryTable query() {
		return query;
	}
	
	public static Periodic periodic() {
		return periodic;
	}
	
	public static Program program(String name) {
		return programs.containsKey(name) ? programs.get(name) : null;
	}
	
	public static void program(String name, Program program) {
		programs.put(name, program);
	}
	
	public static void install(String owner, String file) {
		final TupleSet compilation = new TupleSet(Compiler.compiler.name());
		compilation.add(new Tuple(null, owner, file, null));
		schedule(compilation, "runtime");
	}
	
	public static void schedule(final TupleSet tuples, final String program) {
		synchronized (driver) {
			driver.task(new Driver.Task() {
				public TupleSet tuples() {
					return tuples;
				}

				public String program() {
					return program;
				}
			});
		}
	}
	
	public static void bootstrap() {
		if (initialized) return;
		
		try {
			Table.initialize();
			Compiler.initialize();
			query      = new QueryTable();
			evaluator  = new Evaluate();
			schedule   = new Schedule();
			clock      = new Clock("localhost");
			periodic   = new Periodic(schedule);
			log        = new Log(java.lang.System.err);
			programs   = new Hashtable<String, Program>();
			
			p2.lang.Compiler compiler = new p2.lang.Compiler("system", RUNTIME);
			compiler.program().plan();
			clock.insert(clock.time(0L), null);
			
			driver = new Driver(program("runtime"), schedule, periodic, clock);
			
			for (String file : Compiler.FILES) {
				install("system", file);
			}
			
			system = new Thread(driver);
			system.start();
		} catch (Exception e) {
			e.printStackTrace();
			java.lang.System.exit(1);
		}
		initialized = true;
	}
	
	public static void main(String[] args) {
		bootstrap();
		for (int i = 0; i < args.length; i++)
			install("user", args[i]);
	}
}
