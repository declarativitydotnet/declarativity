package jol.core;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.Timer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import jol.exec.Query.QueryTable;
import jol.lang.Compiler;
import jol.lang.Compiler.CompileTable;
import jol.lang.plan.Program;
import jol.net.Network;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.StasisTable;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Catalog;

/**
 * The system runtime.
 * Contains a reference to all state in an instance of the
 * OverLog library. 
 * A call to the {@link Runtime#create(int)} will create
 * a new runtime object and run it in a separate thread.
 * 
 * Implements the {@link System} interface through which
 * outside programs interact with the OverLog library.
 */
public class Runtime implements System {
	/** Used to grab a quick identifier. */
	private static Long idgenerator = 0L;
	
	/** @return A new unique system identifier. */
	public static Long idgen() {
		return idgenerator++;
	}
	
	/** The thread that the runtime executes in.
	 * Created and started in {#link #create(int)}. */
	private Thread thread;
	
	/** The system catalog contain all table references. */
	private Catalog catalog;
	
	/** The network object that manages communication over
	 * various protocols. */
	private Network network;
	
	/** The driver for the the query processor engine. */
	private Driver driver;
	
	/** A table that contains all tuples waiting to be
	 * executed by the query processor {@link Driver}. */
	private Schedule schedule;
	
	/** The system logical clock. */
	private Clock clock;
	
	/** The system physical clock. */
	private Timer timer;
	
	/** An executor for async queries. */
	private ExecutorService executor;
	
	/** Creates a new runtime. Called from {#link #create(int)}. */
	private Runtime() {
		this.catalog = Table.initialize(this);
		Compiler.initialize(this);
		StasisTable.initializeStasis(this);
		
		this.schedule = new Schedule(this);
		this.clock    = new Clock(this, "localhost");
		this.catalog.register(this.schedule);
		this.catalog.register(this.clock);
		
		this.catalog.register(new QueryTable(this));
		this.catalog.register(new Periodic(this, schedule));
		this.catalog.register(new Log(this, java.lang.System.err));

		this.executor   = Executors.newFixedThreadPool(java.lang.Runtime.getRuntime().availableProcessors());
		this.network    = new Network(this);
		this.driver     = new Driver(this, schedule, clock, executor);
		this.thread     = new Thread(driver);
		this.timer      = new Timer("Timer", true);
	}
	
	public void shutdown() {
		synchronized (driver) {
			this.executor.shutdown();
			this.thread.interrupt();
			if (this.network != null) {
				this.network.shutdown();
			}
			StasisTable.deinitializeStasis();
		}
	}
	
	/**
	 * @return The query processor (Driver) thread.
	 */
	public Thread thread() {
		return this.thread;
	}

	/**
	 * @return The system catalog {@link Catalog}.
	 */
	public Catalog catalog() {
		return this.catalog;
	}
	
	/**
	 * @return The network manager {@link Network}.
	 */
	public Network network() {
		return this.network;
	}
	
	/**
	 * @return The system clock {@link Clock}.
	 */
	public Clock clock() {
		return this.clock;
	}
	
	/**
	 * Get the system physical timer clock.
	 * @return The system timer.
	 */
	public Timer timer() {
		return this.timer;
	}

	/**
	 * Retrieve the program object of an install program.
	 * @param name The name of the program {@link Program}.
	 * @return The program object or null if !exists
	 */
	public Program program(String name) {
		try {
			TupleSet program = catalog().table(new TableName(Table.GLOBALSCOPE, "program")).primary().lookupByKey(name);
			if (program.size() == 0) return null;
			Tuple tuple = program.iterator().next();
			return (Program) tuple.value(Program.ProgramTable.Field.OBJECT.ordinal());
		} catch (Exception e) {
			return null;
		}
	}
	
	/**
	 * Install the program contained in the given file under the given owner.
	 * @param owner The owner of the program.
	 * @param file The file containing the program text.
	 */
	public void install(String owner, URL file) throws UpdateException {
		TupleSet compilation = new TupleSet(CompileTable.TABLENAME);
		compilation.add(new Tuple(null, owner, file.toString(), null));
		schedule("runtime", CompileTable.TABLENAME, compilation, new TupleSet(CompileTable.TABLENAME));
	}
	
	/**
	 * Uninstall a program.
	 * @param name The program name.
	 */
	public void uninstall(String name) throws UpdateException {
		TupleSet uninstall = new TupleSet(new TableName("compiler", "uninstall"), new Tuple(name, true));
		schedule("compile", uninstall.name(), uninstall, null);
	}
	
	/**
	 *  Schedule a set of insertion/deletion tuples.
	 *  @param program The program that should execute the delta tuples.
	 *  @param name The table name w.r.t. the tuple insertions/deletions.
	 *  @param insertions The set of tuples to be inserted and deltas executed.
	 *  @param deletions The set of tuples to be deleted and deltas executed.
	 */
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
	
	/**
	 * Creates a new runtime object that listens on the given network port.
	 * @param port The network port that this runtime listens on.
	 * @return A new runtime object.
	 * @throws P2RuntimeException If something went wrong during bootstrap.
	 */
	public static System create(int port) throws P2RuntimeException {
		try {
			Runtime runtime = new Runtime();
			URL runtimeFile = ClassLoader.getSystemClassLoader().getResource("jol/core/runtime.olg");
			if (runtimeFile == null) {
			    throw new P2RuntimeException("Could not load jol/core/runtime.olg.");
			}
			Compiler compiler = new Compiler(runtime, "system", runtimeFile);
			compiler.program().plan();
			runtime.driver.runtime(runtime.program("runtime"));
			/* Install compiler files first so that further programs
			 * will be evaluated by the compiler (i.e., they will be stratified). */
			for (URL file : Compiler.FILES) {
				runtime.install("system", file);
			}
			runtime.network.install(port);
			runtime.thread.start();

			return runtime;
		} catch (Exception e) {
			e.printStackTrace();
			throw new P2RuntimeException(e.toString());
		}
	}
	
	public static void main(String[] args) throws UpdateException, MalformedURLException, NumberFormatException, P2RuntimeException {

		if (args.length < 2) {
			java.lang.System.out.println("Usage: jol.core.Runtime port program");
			java.lang.System.exit(1);
		}
		
		// Initialize the global Runtime
		Runtime runtime = (Runtime) Runtime.create(Integer.parseInt(args[0]));
		for (int i = 1; i < args.length; i++) {
			URL url = new URL("file", "", args[i]);
			runtime.install("user", url);
		}
	}
}
