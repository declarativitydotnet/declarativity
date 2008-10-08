package jol.core;

import java.net.MalformedURLException;
import java.net.URL;
import jol.exec.Query.QueryTable;
import jol.lang.Compiler;
import jol.lang.Compiler.CompileTable;
import jol.lang.plan.Program;
import jol.net.Network;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Catalog;

public class Runtime implements System {
	private static Runtime runtime;
	
	private static Long idgenerator = 0L;
	
	public static Long idgen() {
		return idgenerator++;
	}
	
	private Thread thread;
	
	private Catalog catalog;
	
	private Network network;
	
	private Driver driver;
	
	private Schedule schedule;
	
	/** The system logical clock. */
	private Clock clock;
	
	Runtime() {
		this.catalog = Table.initialize(this);
		Compiler.initialize(this);

		this.schedule = new Schedule(this);
		this.clock    = new Clock(this, "localhost");
		this.catalog.register(this.schedule);
		this.catalog.register(this.clock);
		
		this.catalog.register(new QueryTable(this));
		this.catalog.register(new Periodic(this, schedule));
		this.catalog.register(new Log(this, java.lang.System.err));
		
		this.network    = new Network(this);
		this.driver     = new Driver(this, schedule, clock);
		this.thread     = new Thread(driver);
	}
	
	public Thread thread() {
		return this.thread;
	}
	
	public Catalog catalog() {
		return this.catalog;
	}
	
	public Network network() {
		return this.network;
	}
	
	public Clock clock() {
		return this.clock;
	}
	
	public Program program(String name) {
		try {
			TupleSet program = catalog().table(new TableName(Table.GLOBALSCOPE, "program")).primary().lookup(name);
			if (program.size() == 0) return null;
			Tuple tuple = program.iterator().next();
			return (Program) tuple.value(Program.ProgramTable.Field.OBJECT.ordinal());
		} catch (Exception e) {
			return null;
		}
	}
	
	public void install(String owner, URL file) throws UpdateException {
		TupleSet compilation = new TupleSet(CompileTable.TABLENAME);
		compilation.add(new Tuple(null, owner, file.toString(), null));
		schedule("runtime", CompileTable.TABLENAME, compilation, new TupleSet(CompileTable.TABLENAME));
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
	
	public void bootstrap(int port) {
		try {
			try {
				URL runtimeFile = ClassLoader.getSystemClassLoader().getResource("jol/core/runtime.olg");
				Compiler compiler = new Compiler(this, "system", runtimeFile);
				compiler.program().plan();
				driver.runtime(program("runtime"));
				network.install(port);
				thread.start();
			} catch (Exception e) {
				throw new P2RuntimeException(e.toString());
			}

			for (URL file : Compiler.FILES) {
				install("system", file);
			}
		} catch (Exception e) {
			java.lang.System.err.println("BOOTSTRAP ERROR");
			e.printStackTrace();
			java.lang.System.exit(1);
		}
	}
	
	public static void main(String[] args) throws UpdateException, MalformedURLException {
		if (args.length != 2) {
			java.lang.System.out.println("Usage: jol.core.Runtime port program");
			java.lang.System.exit(1);
		}
		java.lang.System.err.println(ClassLoader.getSystemClassLoader().getResource("jol/core/runtime.olg"));
		
		// Initialize the global Runtime
		runtime = (Runtime) SystemFactory.makeSystem();
		runtime.bootstrap(Integer.parseInt(args[0]));
		for (int i = 1; i < args.length; i++) {
			URL url = new URL("file", "", args[i]);
			runtime.install("user", url);
		}
	}
}
