package p2.lang;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;

import p2.core.Periodic;
import p2.lang.parse.Parser;
import p2.lang.parse.TypeChecker;
import p2.lang.plan.*;
import p2.lang.plan.Program.ProgramTable;
import p2.lang.plan.Assignment.AssignmentTable;
import p2.lang.plan.Fact.FactTable;
import p2.lang.plan.Function.TableFunction;
import p2.lang.plan.Predicate.PredicateTable;
import p2.lang.plan.Rule.RuleTable;
import p2.lang.plan.Selection.SelectionTable;
import p2.lang.plan.Watch.WatchTable;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.EventTable;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;
import p2.types.table.TableName;

import xtc.Constants;
import xtc.tree.Node;
import xtc.util.Runtime;

/**
 * The driver for processing the Overlog language.
 */
public class Compiler {
	public static final URL[] FILES = {
			ClassLoader.getSystemClassLoader().getResource("p2/lang/compile.olg"),
			ClassLoader.getSystemClassLoader().getResource("p2/lang/stratachecker.olg")
		};

	public static class CompileTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "compiler");
		public static final Key PRIMARY_KEY = new Key(0);
		public enum Field {NAME, OWNER, FILE, PROGRAM};

		public static final Class[] SCHEMA = {
				String.class, // Program name
				String.class, // Program owner
				String.class, // Program file
				Program.class // The program object
		};
		
		private p2.core.Runtime context;

		public CompileTable(p2.core.Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
			this.context = context;
		}

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
				} catch (P2RuntimeException e) {
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

	public static final void initialize(p2.core.Runtime context) {
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

	private p2.core.Runtime context;
	
	private String owner;

	private Program program;

	private Runtime runtime = new Runtime();

	/** Create a new driver for Overlog. */
	public Compiler(p2.core.Runtime context, String owner, URL input) throws P2RuntimeException {
		this.context = context;
		this.owner = owner;
		this.runtime = new Runtime();

		Node ast = parse(input);
		process(ast);

		if (this.runtime.errorCount() > 0) {
			for (Table table : this.program.definitions()) {
				try {
					context.catalog().drop(table.name());
				} catch (UpdateException e) {
					e.printStackTrace();
				}
			}
			throw new P2RuntimeException("Compilation of program "
					+ program.name() + " resulted in "
					+ this.runtime.errorCount() + " errors.");
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
			
			while ((n = isr.read(buf, 0, buf.length)) != -1)
			{
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
		TypeChecker typeChecker = new TypeChecker(context, this.runtime, this.program);
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
			if (clause.getName().equals("Table")) {
				typeChecker.analyze(clause);
				if (runtime.errorCount() > 0)
					return;
			} else if (clause.getName().equals("Event")) {
				typeChecker.analyze(clause);
				if (runtime.errorCount() > 0)
					return;
			}
		}

		/* All programs define a local periodic event table. */
		TableName periodic = new TableName(program.name(), "periodic");
		Table eventTable = new EventTable(periodic, new TypeList(Periodic.SCHEMA));
		context.catalog().register(eventTable);
		program.definition(eventTable);

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
						List<Watch> watches = (List<Watch>) clause.getProperty(Constants.TYPE);
						for (Watch watch : watches) {
							watch.set(context, this.program.name());
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
