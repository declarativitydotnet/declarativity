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
	private static String RUNTIME = "src/p2/core/runtime.olg";
	
	private static QueryTable query;
	
	private static CompileTable compile;
	
	private static Evaluate evaluator;
	
	private static Driver driver;
	
	private static Schedule schedule;
	
	/** The system logical clock. */
	private static Clock clock;
	
	private static Periodic periodic;
	
	private static Log log;
	
	private static Hashtable<String, Program> programs;
	
	public static void initialize() {
		Table.initialize();
		query      = new QueryTable();
		compile    = new CompileTable();
		evaluator  = new Evaluate();
		schedule   = new Schedule();
		clock      = new Clock("localhost");
		periodic   = new Periodic(schedule);
		log        = new Log(java.lang.System.err);
		programs   = new Hashtable<String, Program>();
	}
	
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
		synchronized (driver) {
			final TupleSet compilation = new TupleSet(compile.name());
			compilation.add(new Tuple(null, owner, file, null));
			driver.task(new Driver.Task() {
				public TupleSet tuples() {
					return compilation;
				}
			});
		}
	}
	
	private static void bootstrap() {
		try {
			p2.lang.Compiler compiler = new p2.lang.Compiler("system", RUNTIME);
			compiler.program().plan();
			clock.insert(clock.time(0L), null);
			
			driver = new Driver(program("runtime"), schedule, periodic, clock);
			
			for (String file : Compiler.FILES) {
				TupleSet compilation = new TupleSet(compile.name());
				compilation.add(new Tuple(null, "system", file, null));
				driver.evaluate(compilation);
			}
		} catch (Exception e) {
			e.printStackTrace();
			java.lang.System.exit(1);
		}
	}
	
	public static void main(String[] args) {
		initialize();
		bootstrap();
		for (int i = 0; i < args.length; i++)
			install("user", args[i]);
		driver.run();
	}
}
