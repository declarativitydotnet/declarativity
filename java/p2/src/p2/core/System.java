package p2.core;

import java.util.Hashtable;
import p2.exec.Planner;
import p2.exec.Schedule;
import p2.exec.Compiler.CompileTable;
import p2.exec.Query.QueryTable;
import p2.types.table.Table;

public class System {
	private static String RUNTIME = "./runtime.olg";
	
	private static QueryTable query;
	
	private static CompileTable compile;
	
	private static Planner planner;
	
	private static Hashtable<String, Program> programs;
	
	private static Driver driver;
	
	public static void initialize() {
		Table.initialize();
		query    = new QueryTable();
		compile  = new CompileTable();
		planner  = new Planner();
		programs = new Hashtable<String, Program>();
	}
	
	public static QueryTable query() {
		return query;
	}
	
	public static Program program(String name) {
		return programs.contains(name) ? programs.get(name) : null;
	}
	
	private static void bootstrap() {
		p2.exec.Compiler compiler = new p2.exec.Compiler(RUNTIME);
		driver = new Driver(compiler.program(), new Schedule());
	}
	
}
