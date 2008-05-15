package p2.lang;

import java.io.File;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;

import p2.lang.parse.Parser;
import p2.lang.parse.TypeChecker;
import p2.lang.plan.*;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.EventTable;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

import xtc.Constants;
import xtc.parser.ParseException;
import xtc.tree.Node;
import xtc.util.Tool;


/**
 * The driver for processesing the Overlog language.
 */
public class Compiler extends Tool {

	public static class CompileTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0);

		public enum Field{NAME, OWNER, FILE, PROGRAM};
		public static final Class[] SCHEMA = {
			String.class,  // Program name
			String.class,  // Program owner
			String.class,  // Program file
			Program.class  // The program object
		};

		public CompileTable() {
			super("compiler", PRIMARY_KEY, new TypeList(SCHEMA));
		}

		protected boolean insert(Tuple tuple) throws UpdateException {
			String name  = (String) tuple.value(Field.NAME.ordinal());
			String owner = (String) tuple.value(Field.OWNER.ordinal());
			String file  = (String) tuple.value(Field.FILE.ordinal());
			Compiler compiler = new Compiler(name, owner, file);
			tuple.value(Field.PROGRAM.ordinal(), compiler.program());
			return super.insert(tuple);
		}
	}


	private String file;
	
	private Program program;
	
	private TypeChecker typeChecker;

	/** Create a new driver for Overlog. */
	public Compiler(String name, String owner, String file) {
		this.program = new Program(name, owner);
		this.file = file;
		typeChecker = new TypeChecker(this.runtime);
		String[] args = new String[2];
		args[0] = "-no-exit";
		args[1] = file;
		run(args);
	}
	
	public Program program() {
		return this.program;
	}
	
	public String getName() {
		return "OverLog Compiler";
	}

	public String getCopy() {
		return Constants.FULL_COPY;
	}

	public void init() {
		super.init();
		runtime.
		bool("optionTypeAnalyze", "optionTypeAnalyze", false,
		"Analyze the program's AST.").
		bool("printAST", "printAST", false,
		"Print the program's AST in generic form.").
		bool("printSource", "printSource", false,
		"Print the program's AST in source form.").
		bool("printSymbolTable", "printSymbolTable", false,
		"Print the program's symbol table.");
	}

	public Node parse(Reader in, File file) throws IOException, ParseException {
		try {
			Parser parser = new Parser(in, file.toString(), (int)file.length());
			return (Node)parser.value(parser.pProgram(0));
		} catch (ParseException e) {
			System.err.println(e.getMessage());
			e.printStackTrace();
			System.exit(0);
		}
		return null;
	}

	public void process(Node node) {
		// Perform type checking.
		runtime.console().format(node).pln().flush();
		typeChecker.prepare();
		
		/* First evaluate all import statements. */
		for (Node clause : node.<Node>getList(0)) {
			if (clause.getName().equals("Import")) {
				typeChecker.analyze(clause);
			}
		}

		/* Next evaluate all table and event declarations. */ 
		for (Node clause : node.<Node>getList(0)) {
			if (clause.getName().equals("Table")) {
				typeChecker.analyze(clause);
				this.program.definition((Table)clause.getProperty(Constants.TYPE));
			}
			else if (clause.getName().equals("Event")) {
				typeChecker.analyze(clause);
				this.program.definition((Table)clause.getProperty(Constants.TYPE));
			}
		}

		/* Evaluate all other clauses. */
		for (Node clause : node.<Node>getList(0)) {
			if (clause.getName().equals("Rule") ||
			    clause.getName().equals("Fact") ||
			    clause.getName().equals("Watch")) {
				typeChecker.analyze(clause);
			}
		}
		if (runtime.test("printAST")) {
			runtime.console().format(node).pln().flush();
		}
	}
}
