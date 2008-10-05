package p2.core;

import java.net.MalformedURLException;
import java.net.URL;
import p2.exec.Query.QueryTable;
import p2.lang.Compiler;
import p2.lang.plan.Program;
import p2.net.Network;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.Table;
import p2.types.table.TableName;

public class Runtime implements System {
	private static Runtime runtime;
	
	private static Long idgenerator = 0L;
	
	public static Long idgen() {
		return idgenerator++;
	}
	
	private Thread thread;
	
	private Network network;
	
	private QueryTable query;
	
	private Driver driver;
	
	private Schedule schedule;
	
	/** The system logical clock. */
	private Clock clock;
	
	private Periodic periodic;
	
	private Log log;
	
	private Runtime() {
		query      = new QueryTable();
		schedule   = new Schedule();
		clock      = new Clock("localhost");
		periodic   = new Periodic(schedule);
		log        = new Log(java.lang.System.err);
		network    = new Network();
		driver     = new Driver(schedule, periodic, clock);
		thread     = new Thread(driver);
	}
	
	public void start(Integer port) throws P2RuntimeException {
		try {
			URL runtimeFile = ClassLoader.getSystemClassLoader().getResource("p2/core/runtime.olg");
			p2.lang.Compiler compiler = new p2.lang.Compiler("system", runtimeFile);
			compiler.program().plan();
			driver.runtime(program("runtime"));
			network.install(port);
			thread.start();
		} catch (Exception e) {
			throw new P2RuntimeException(e.toString());
		}
	}
	
	public Thread thread() {
		return this.thread;
	}
	
	public Clock clock() {
		return this.clock;
	}
	
	public QueryTable query() {
		return this.query;
	}
	
	public Periodic periodic() {
		return this.periodic;
	}
	
	public Program program(String name) {
		try {
			TupleSet program = Table.table(new TableName(Table.GLOBALSCOPE, "program")).primary().lookup(name);
			if (program.size() == 0) return null;
			Tuple tuple = program.iterator().next();
			return (Program) tuple.value(Program.ProgramTable.Field.OBJECT.ordinal());
		} catch (Exception e) {
			return null;
		}
	}
	
	public void install(String owner, URL file) throws UpdateException {
		TupleSet compilation = new TupleSet(Compiler.compiler.name());
		compilation.add(new Tuple(null, owner, file.toString(), null));
		schedule("runtime", Compiler.compiler.name(), compilation, new TupleSet(Compiler.compiler.name()));
	}
	
	public void uninstall(String program) throws UpdateException {
		TupleSet uninstall = new TupleSet(new TableName("compiler", "uninstall"), new Tuple(program, true));
		schedule("compile", uninstall.name(), uninstall, null);
	}
	
	public void schedule(final String program, final TableName name, 
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
	
	public static Runtime runtime() {
		return runtime;
	}
	
	public static System bootstrap(Integer port) {
		try {
			Table.initialize();
			Compiler.initialize();
			runtime = new Runtime();
			runtime.start(port);

			for (URL file : Compiler.FILES) {
				runtime.install("system", file);
			}
			
			return runtime;
		} catch (Exception e) {
			java.lang.System.err.println("BOOTSTRAP ERROR");
			e.printStackTrace();
			java.lang.System.exit(1);
		}
		return null;
	}
	
	public static void main(String[] args) throws UpdateException, InterruptedException, MalformedURLException {
		if (args.length != 2) {
			java.lang.System.out.println("Usage: p2.core.System port program");
			java.lang.System.exit(1);
		}
		java.lang.System.err.println(ClassLoader.getSystemClassLoader().getResource("p2/core/runtime.olg"));
		Runtime system = (Runtime) bootstrap(Integer.parseInt(args[0]));
		for (int i = 1; i < args.length; i++) {
			URL url = new URL("file", "", args[i]);
			system.install("user", url);
		}
		system.thread().join();
	}
}
