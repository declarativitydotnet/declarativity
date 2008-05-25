package p2.core;

import java.util.Hashtable;

import p2.core.Driver.Evaluate;
import p2.exec.Query.QueryTable;
import p2.lang.Compiler;
import p2.lang.Compiler.CompileTable;
import p2.lang.plan.Program;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.PlannerException;
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
	
	private static Hashtable<String, Program> programs;
	
	public static void initialize() {
		Table.initialize();
		query      = new QueryTable();
		compile    = new CompileTable();
		evaluator  = new Evaluate();
		schedule   = new Schedule();
		clock      = new Clock("localhost");
		programs   = new Hashtable<String, Program>();
	}
	
	public static Evaluate evaluator() {
		return evaluator;
	}
	
	public static QueryTable query() {
		return query;
	}
	
	public static Program program(String name) {
		return programs.containsKey(name) ? programs.get(name) : null;
	}
	
	private static void bootstrap() {
		java.lang.System.err.println("BOOSTRAP");
		try {
			p2.lang.Compiler compiler = new p2.lang.Compiler("runtime", "system", RUNTIME);
			compiler.program().plan();
			java.lang.System.err.println(compiler.program().toString());
			programs.put(compiler.program().name(), compiler.program());
			driver = new Driver(compiler.program(), schedule, clock);
		} catch (PlannerException e) {
			java.lang.System.exit(1);
		} catch (P2RuntimeException e) {
			java.lang.System.exit(1);
		}
		
		Thread runtime = new Thread(driver);
		runtime.start();
		try {
			runtime.join();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public static void main(String[] args) {
		initialize();
		bootstrap();
	}
}
