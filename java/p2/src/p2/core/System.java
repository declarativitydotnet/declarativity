package p2.core;

import java.util.Hashtable;

import p2.core.Driver.DriverTable;
import p2.exec.Query.QueryTable;
import p2.lang.Compiler;
import p2.lang.Compiler.CompileTable;
import p2.lang.plan.Program;
import p2.types.table.Table;

public class System {
	private static String RUNTIME = "src/p2/core/runtime.olg";
	
	private static QueryTable query;
	
	private static CompileTable compile;
	
	private static DriverTable driverTable;
	
	private static Driver driver;
	
	private static Schedule schedule;
	
	private static Hashtable<String, Program> programs;
	
	public static void initialize() {
		Table.initialize();
		query       = new QueryTable();
		compile     = new CompileTable();
		driverTable = new DriverTable();
		schedule    = new Schedule();
		programs    = new Hashtable<String, Program>();
	}
	
	public static Driver driver() {
		return driver;
	}
	
	public static QueryTable query() {
		return query;
	}
	
	public static Program program(String name) {
		return programs.contains(name) ? programs.get(name) : null;
	}
	
	private static void bootstrap() {
		p2.lang.Compiler compiler = new p2.lang.Compiler();
		driver = new Driver(compiler.program("runtime", "system", RUNTIME), schedule);
	}
	
	public static void main(String[] args) {
		initialize();
		bootstrap();
	}
}
