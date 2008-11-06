package jol.lang;

import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;

import jol.lang.parse.Parser;
import jol.lang.parse.TypeChecker;
import jol.lang.plan.*;
import jol.lang.plan.Program.ProgramTable;
import jol.lang.plan.Assignment.AssignmentTable;
import jol.lang.plan.Fact.FactTable;
import jol.lang.plan.Function.TableFunction;
import jol.lang.plan.Predicate.PredicateTable;
import jol.lang.plan.Rule.RuleTable;
import jol.lang.plan.Selection.SelectionTable;
import jol.lang.plan.Watch.WatchTable;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Aggregation;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;

import xtc.Constants;
import xtc.tree.Node;
import xtc.util.Runtime;

/**
 * The driver for processing the Overlog language.
 */
public class Compiler {
	public static URL[] FILES(jol.core.Runtime.ResourceLoader l) throws JolRuntimeException {
		return new URL[] {
				l.getResource("jol/lang/compile.olg"),
				l.getResource("jol/lang/stratachecker.olg"),
				l.getResource("jol/lang/grappa.olg")
		};
	}
	public static class CompileTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "compiler");
		public static final Key PRIMARY_KEY = new Key(0);
		public enum Field {NAME, OWNER, DEBUGGER, FILE, PROGRAM};

		public static final Class[] SCHEMA = {
				String.class, // Program name
				String.class, // Program owner
				String.class, // Debugger
				String.class, // Program file
				Program.class // The program object
		};
		
		private jol.core.Runtime context;

		public CompileTable(jol.core.Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
			this.context = context;
		}

		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Program program = (Program) tuple.value(Field.PROGRAM.ordinal());
			if (program == null) {
				String owner = (String) tuple.value(Field.OWNER.ordinal());
				String file = (String) tuple.value(Field.FILE.ordinal());
				try {
					URL fileURL = new URL(file);
					Compiler compiler = new Compiler(context, owner, fileURL);
					tuple.value(Field.NAME.ordinal(), compiler.program.name());
					tuple.value(Field.PROGRAM.ordinal(), compiler.program);
				} catch (JolRuntimeException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
					throw new UpdateException(e.toString());
				} catch (MalformedURLException e) {
					e.printStackTrace();
					throw new UpdateException(e.toString());
				}
			}
			return super.insert(tuple);
		}
	}

	public static final void initialize(jol.core.Runtime context) {
		context.catalog().register(new CompileTable(context));
		context.catalog().register(new ProgramTable(context));
		context.catalog().register(new RuleTable(context));
		context.catalog().register(new WatchTable(context));
		context.catalog().register(new FactTable(context));
		context.catalog().register(new PredicateTable(context));
		context.catalog().register(new TableFunction(context));
		context.catalog().register(new SelectionTable(context));
		context.catalog().register(new AssignmentTable(context));
	}

	private jol.core.Runtime context;
	
	private String owner;

	private Program program;

	private final static Runtime runtime = new Runtime(); 

	/** Create a new driver for Overlog. */
	public Compiler(jol.core.Runtime context, String owner, URL input) throws JolRuntimeException {
		this.context = context;
		this.owner = owner;

		synchronized (runtime) {
			Node ast = parse(input);
			process(ast);

			if (runtime.errorCount() > 0) {
				for (Table table : this.program.definitions()) {
					try {
						context.catalog().drop(table.name());
					} catch (UpdateException e) {
						e.printStackTrace();
					}
				}
				throw new JolRuntimeException("Compilation of program "
						+ program.name() + " resulted in "
						+ runtime.errorCount() + " errors.");
			}
		}
	}
	

	public Program program() {
		return this.program;
	}

	public Node parse(URL input) {
		try {
			String inputString = readURL(input);
			Reader inputReader = new StringReader(inputString);
			Parser parser = new Parser(inputReader, input.toString(), inputString.length());
			return (Node) parser.value(parser.pProgram(0));
		} catch (Exception e) {
			// XXX: proper error handling
			System.err.println(e.getMessage());
			e.printStackTrace();
			System.exit(0);
		}
		return null;
	}

	/*
	 * Read the content at the specified URL into a String. In theory, this
	 * could be a blocking operation or might return a very large string; in
	 * practice, we use URLs solely to represent elements of a JAR file, so it
	 * should be a local filesystem read of a relatively small file.
	 */
	private String readURL(URL url) throws IOException {
		InputStreamReader isr = new InputStreamReader(url.openStream());
		StringBuilder sb = new StringBuilder();

		try {
			char[] buf = new char[512];
			int n;
			
			while ((n = isr.read(buf, 0, buf.length)) != -1) {
				sb.append(buf, 0, n);
			}
		}
		finally {
			isr.close();
		}

		return sb.toString();
	}

	public void process(Node node) {
		String name = node.getString(0);

		// Perform type checking.
		// runtime.console().format(node).pln().flush();
		this.program = new Program(context, name, owner);
		TypeChecker typeChecker = new TypeChecker(context, Compiler.runtime, this.program);
		typeChecker.prepare();

		/* First evaluate all import statements. */
		for (Node clause : node.getNode(1).<Node> getList(0)) {
			if (clause.getName().equals("Import")) {
				typeChecker.analyze(clause);
				if (runtime.errorCount() > 0)
					return;
			}
		}

		/* Next evaluate all table and event declarations. */
		for (Node clause : node.getNode(1).<Node> getList(0)) {
			if (clause.getName().equals("Table") ||
			    clause.getName().equals("Event") ||
			    clause.getName().equals("Timer")) {
				typeChecker.analyze(clause);
				if (runtime.errorCount() > 0)
					return;
			}
		}

		/* Evaluate all other clauses. */
		for (Node clause : node.getNode(1).<Node> getList(0)) {
			if (clause.getName().equals("Rule")
					|| clause.getName().equals("Fact")
					|| clause.getName().equals("Load")
					|| clause.getName().equals("Watch")) {
				typeChecker.analyze(clause);
				if (runtime.errorCount() > 0)
					return;
				try {
					if (clause.getName().equals("Watch")) {
						List watches = (List) clause.getProperty(Constants.TYPE);
						for (Object watch : watches) {
							((Watch)watch).set(context, this.program.name());
						}
					} else {
						Clause c = (Clause) clause.getProperty(Constants.TYPE);
						if (c == null) continue;
						c.set(context, this.program.name());
					}
				} catch (UpdateException e) {
					e.printStackTrace();
					runtime.error(e.toString());
				}
			}
		}
	}
}
