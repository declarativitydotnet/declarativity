package p2.core;

import java.net.URL;
import java.util.Hashtable;

import p2.core.Driver.Task;
import p2.exec.Query.QueryTable;
import p2.lang.Compiler;
import p2.lang.plan.Program;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.UpdateException;
import p2.types.table.Table;
import p2.types.table.TableName;

public class System {
	private static boolean initialized = false;
	
	private static Long idgenerator = 0L;
	
	public static Long idgen() {
		return idgenerator++;
	}
	
	private static Thread system;
	
	private static p2.net.Network netManager;
	
	private static QueryTable query;
	
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
	
	public static void install(String owner, String file) throws UpdateException {
		TupleSet compilation = new TupleSet(Compiler.compiler.name());
		compilation.add(new Tuple(null, owner, file, null));
		schedule("runtime", Compiler.compiler.name(), compilation, new TupleSet(Compiler.compiler.name()));
	}
	
	public static void schedule(final String program, final TableName name, 
			                    final TupleSet insertions, final TupleSet deletions) throws UpdateException {
		synchronized (driver) {
			if (program.equals("runtime")) {
			driver.task(new Driver.Task() {
				public TupleSet insertions() {
					return insertions;
				}
				
				public TupleSet deletions() {
					return deletions;
				}

				public String program() {
					return program;
				}

				public TableName name() {
					return name;
				}
			});
			}
			else {
				Tuple tuple = new Tuple(clock.current() + 1L, program, name, 
						                insertions, deletions);
				schedule.force(tuple);
			}
			driver.notify();
		}
	}
	
	public static void bootstrap(Integer port, String program) {
		if (initialized) return;
		
		try {
			Table.initialize();
			Compiler.initialize();
			query      = new QueryTable();
			schedule   = new Schedule();
			clock      = new Clock("localhost");
			periodic   = new Periodic(schedule);
			log        = new Log(java.lang.System.err);
			programs   = new Hashtable<String, Program>();
			netManager = new p2.net.Network();
			driver = new Driver(schedule, periodic, clock);
			
	        URL runtimeFile = ClassLoader.getSystemClassLoader().getResource("p2/core/runtime.olg");
			p2.lang.Compiler compiler = new p2.lang.Compiler("system", runtimeFile.getPath());
			compiler.program().plan();
			driver.runtime(program("runtime"));
			
			for (String file : Compiler.FILES) {
				install("system", file);
			}
			
			netManager.install(port);
			
			install("user", program);
			
			system = new Thread(driver);
			system.start();
			system.join();
		} catch (Exception e) {
			java.lang.System.err.println("BOOTSTRAP ERROR");
			e.printStackTrace();
			java.lang.System.exit(1);
		}
		initialized = true;
	}
	
	public static void main(String[] args) throws UpdateException {
		java.lang.System.err.println(ClassLoader.getSystemClassLoader().getResource("p2/core/runtime.olg"));
		bootstrap(Integer.parseInt(args[0]), args[1]);
	}

}
