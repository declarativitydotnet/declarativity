package p2.lang.parse;

import java.io.File;
import java.io.IOException;
import java.io.Reader;

import p2.lang.plan.*;

import xtc.Constants;
import xtc.parser.ParseException;
import xtc.tree.Node;
import xtc.util.Tool;


/**
 * The driver for processesing the Overlog language.
 */
public class Overlog extends Tool {
	
	private TypeChecker typeChecker;
	
	private Program program;

  /** Create a new driver for Overlog. */
  public Overlog() {
	  typeChecker = new TypeChecker(this.runtime);
  }

  public String getName() {
    return "xtc Overlog Driver";
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
    Parser parser = new Parser(in, file.toString(), (int)file.length());
    this.program = new Program(file.getName());
    return (Node)parser.value(parser.pProgram(0));
  }

  public void process(Node node) {
	  // Perform type checking.
	  typeChecker.prepare();
	  
	  /* First evaluate all table and event declarations. */ 
	  for (Node clause : node.<Node>getList(0)) {
		  if (clause.getName().equals("Table")) {
			  typeChecker.analyze(clause);
			  this.program.table((p2.lang.plan.Table)clause.getProperty(Constants.TYPE));
		  }
		  else if (clause.getName().equals("Event")) {
			  typeChecker.analyze(clause);
			  this.program.event((Event)clause.getProperty(Constants.TYPE));
		  }
	  }
	  
	  /* Evaluate all other clauses. */
	  for (Node clause : node.<Node>getList(0)) {
		  if (clause.getName().equals("Rule")) {
			  typeChecker.analyze(clause);
			  this.program.rule((Rule)clause.getProperty(Constants.TYPE));
		  }
		  else if (clause.getName().equals("Fact")) {
			  typeChecker.analyze(clause);
			  this.program.fact((Fact)clause.getProperty(Constants.TYPE));
		  }
		  else if (clause.getName().equals("Watch")) {
			  typeChecker.analyze(clause);
			  this.program.watch((Watch)clause.getProperty(Constants.TYPE));
		  }
	  }
	  System.err.println(program);
	  
	  if (runtime.test("printAST")) {
		  runtime.console().format(node).pln().flush();
	  }
  }

  /**
   * Run the driver with the specified command line arguments.
   *
   * @param args The command line arguments.
   */
  public static void main(String[] args) {
	  p2.core.System.initialize();
      new Overlog().run(args);
  }
}
