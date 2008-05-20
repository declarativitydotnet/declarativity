package p2.core;

import java.util.Hashtable;

import p2.core.Driver.DriverTable;
import p2.exec.Query.QueryTable;
import p2.lang.Compiler;
import p2.lang.Compiler.CompileTable;
import p2.lang.plan.Program;
import p2.types.exception.PlannerException;
import p2.types.table.Table;

public class System {
	private static String RUNTIME = "src/p2/core/runtime.olg";
	
	private static QueryTable query;
	
	private static CompileTable compile;
	
	private static DriverTable driverTable;
	
	private static Driver driver;
	
	private static Schedule schedule;
	
	/** The system logical clock. */
	private static Clock clock;
	
	private static Hashtable<String, Program> programs;
	
	public static void initialize() {
		Table.initialize();
		query       = new QueryTable();
		compile     = new CompileTable();
		driverTable = new DriverTable();
		schedule    = new Schedule();
		clock       = new Clock("localhost");
		programs    = new Hashtable<String, Program>();
	}
	
	public static Driver driver() {
		return driver;
	}
	
	public static QueryTable query() {
		return query;
	}
	
	public static Program program(String name) {
		return programs.containsKey(name) ? programs.get(name) : null;
	}
	
	private static void bootstrap() {
		java.lang.System.err.println("BOOSTRAP");
		p2.lang.Compiler compiler = new p2.lang.Compiler("runtime", "system", RUNTIME);
		try {
			compiler.program().plan();
			java.lang.System.err.println(compiler.program().toString());
		} catch (PlannerException e) {
			e.printStackTrace();
			java.lang.System.exit(1);
		}
		programs.put(compiler.program().name(), compiler.program());
		driver = new Driver(compiler.program(), schedule, clock);
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
