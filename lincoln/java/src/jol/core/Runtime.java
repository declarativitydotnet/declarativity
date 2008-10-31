package jol.core;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.Collection;
import java.util.Enumeration;
import java.util.List;
import java.util.Timer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.servlet.ServletContext;

import jol.exec.Query.QueryTable;
import jol.lang.Compiler;
import jol.lang.Compiler.CompileTable;
import jol.lang.plan.Program;
import jol.net.Network;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.StasisTable;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.types.table.Table.Catalog;

/**
 * The system runtime. Contains a reference to all state in an instance of the
 * OverLog library. A call to the {@link Runtime#create(int)} will create a new
 * runtime object. The runtime can either be used synchronously by calling
 * {@link #evaluate()}, or asynchronously in its own thread by calling
 * {@link Runtime#start()}.
 * 
 * Implements the {@link System} interface through which outside programs
 * interact with the OverLog library.
 */
public class Runtime implements System {
	/** Used to grab a quick identifier. */
	private static Long idgenerator = 0L;

	private static jol.core.Runtime.ResourceLoader loader;
	
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
		this.catalog.register(new Log(this, java.lang.System.err));

		this.executor   = Executors.newFixedThreadPool(java.lang.Runtime.getRuntime().availableProcessors());
		this.network    = new Network(this);
		this.driver     = new Driver(this, schedule, clock, executor);
		this.thread     = new Thread(driver);
		this.timer      = new Timer("Timer", true);
	}
	
	public void evaluate() throws JolRuntimeException {
		// XXX might not be a good idea to bypass this check.
		//		if (this.thread.isAlive()) {
		//			throw new JolRuntimeException("ERROR: can't call evaluate when system has been started!");
		//		}
		synchronized (driver) {
			try {
				this.driver.evaluate();
			} catch (UpdateException e) {
				throw new JolRuntimeException(e.toString());
			}
		}
	}
	public void evaluateTimestamp(RuntimeCallback pre,
								 RuntimeCallback post) throws JolRuntimeException, UpdateException {
		synchronized (driver) {
			// XXX disableInterrupts();
			try {
				this.driver.timestampPrepare();

				call(pre);

				this.driver.timestampEvaluate();

				call(post);
				
			} finally {
				// XXX enableInterrupts();
			}
		}
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
	
	public void start() {
		this.thread.start();
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
	 * Code that wants to install overlog files on the classpath
	 * should call this method instead of using ClassLoader directly.
	 * 
	 * This allows overlog installation in environments like Servlet
	 * containers that provide non-standard ClassLoaders 
	 * 
	 * @param owner The owner of the program
	 * @param resource The name of a program that lives in the classpath,
	 * 				   without a leading '/'.
	 * @throws UpdateException
	 */
	public void install(String owner, String resource)
		throws UpdateException {
		try {
			install(owner, loader.getResource(resource));
		} catch (JolRuntimeException e) {
			// TODO Do we want to wrap this, or propagate JolRuntime exceptions?
			throw new UpdateException("Error loading resource", e);
		}
	}
	
	/**
	 * Install the program contained in the given file under the given owner.
	 * 
	 * This should *not* be used to install overlog files that live in the 
	 * classpath.  The locations of such files should be passed as a string.
	 * 
	 * @param owner The owner of the program.
	 * @param url The location that contains the program text.
	 */
	public void install(String owner, URL url) throws UpdateException {
		TupleSet compilation = new TupleSet(CompileTable.TABLENAME);
		compilation.add(new Tuple(null, owner, url.toString(), null));
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
	public interface RuntimeCallback {
		public void call(Runtime r) throws UpdateException, JolRuntimeException;
	};
	public void call(RuntimeCallback cb) throws UpdateException, JolRuntimeException {
		synchronized (driver) {
			cb.call(this);
		}
	}
	/**
	 * Maps from resource filename to URL.  Unfortunately, we cannot always
	 * rely on the ClassLoader to lookup resources for us.
	 * 
	 *  @see defaultLoader()
	 *  @see servletLoader()
	 */
	public interface ResourceLoader {
		URL getResource(String filename) throws JolRuntimeException;
	};
	/**
	 * Create a resource loader that uses a ServletContext to locate
	 * resources.
	 * 
	 * TODO Keeps pointers to servlet contexts around after getPost() returns,
	 * 		then access them from concurrent threads.  Is that safe?
	 *   
	 * @param c The servlet context that will be used to lookup resources. 
	 * @return A ResourceLoader that can be passed to create().
	 */
	public static ResourceLoader servletLoader(final ServletContext c) {
		return new ResourceLoader() {
			public URL getResource(String file) throws JolRuntimeException {
				try {
					// Is living with the classes?
					URL resource = c.getResource("/WEB-INF/classes/"+file);
					if(resource == null) {
						// Is it at the root of the WAR?
					    resource = c.getResource("/"+file);
					}
					if(resource == null) {
						throw new JolRuntimeException("Could not load " + file
									+ " servlet context root is " + c.getRealPath("/"));
					}
					return resource;
				} catch (MalformedURLException e) {
					throw new JolRuntimeException("Could not load resource", e);
				}
			}
		};
	}
	public static ResourceLoader defaultLoader() {
		return new ResourceLoader() {
			public URL getResource(String arg0) throws JolRuntimeException {
				URL runtimeFile = ClassLoader.getSystemResource(arg0);
				if(runtimeFile == null) {
					throw new JolRuntimeException("Could not load " + arg0);
				}
				return runtimeFile;
			}
		};
	}
	public static System create(int port) throws JolRuntimeException {
		return create(port, defaultLoader());
	}
	/**
	 * Creates a new runtime object that listens on the given network port.
	 * @param port The network port that this runtime listens on.
	 * @return A new runtime object.
	 * @throws JolRuntimeException If something went wrong during bootstrap.
	 */
	public static System create(int port, ResourceLoader l) throws JolRuntimeException {
		try {
			Runtime.loader = l;
			Runtime runtime = new Runtime();
			URL runtimeFile = loader.getResource("jol/core/runtime.olg");
			Compiler compiler = new Compiler(runtime, "system", runtimeFile);
			compiler.program().plan();
			runtime.driver.runtime(runtime.program("runtime"));
			/* Install compiler files first so that further programs
			 * will be evaluated by the compiler (i.e., they will be stratified). */
			for (URL file : Compiler.FILES(loader)) {
				runtime.install("system", file);
			}
			runtime.network.install(port);
			runtime.driver.evaluate();

			return runtime;
		} catch (Exception e) {
			throw new JolRuntimeException("Failed to create JOL runtime instance", e);
		}
	}
	
	public static void main(String[] args) throws UpdateException, MalformedURLException, NumberFormatException, JolRuntimeException {
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
		runtime.evaluate(); // Install program arguments.
		runtime.start();
	}
}
