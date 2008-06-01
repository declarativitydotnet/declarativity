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
	
	private static Hashtable<String, Program> programs;
	
	public static void initialize() {
		Table.initialize();
		query      = new QueryTable();
		compile    = new CompileTable();
		evaluator  = new Evaluate();
		schedule   = new Schedule();
		clock      = new Clock("localhost");
		periodic   = new Periodic(schedule);
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
	
	public static boolean install(String runtime, String name, String owner, String file) {
		synchronized (schedule) {
			TupleSet compilation = new TupleSet(compile.name());
			compilation.add(new Tuple(name, owner, file, null));
			try {
				driver.evaluate(compilation, clock.current());
			} catch (UpdateException e) {
				e.printStackTrace();
				return false;
			}
		}
		return true;
	}
	
	private static void bootstrap(String name, String owner) {
		try {
			Program program = new Program(name, owner);
			p2.lang.Compiler compiler = new p2.lang.Compiler(program, RUNTIME);
			compiler.program().plan();
			clock.insert(clock.time(0L), null);
		} catch (Exception e) {
			e.printStackTrace();
			java.lang.System.exit(1);
		}

		driver = new Driver(program("runtime"), schedule, periodic, clock);
		install("runtime", "compile", "system", Compiler.FILENAME);
	}
	
	public static void main(String[] args) {
		initialize();
		bootstrap("runtime", "system");
		java.lang.System.err.println("ARGS " + args);
		if (args.length > 0) {
			install("runtime", "command", "none", args[0]);
		}
		driver.run();
	}
}
