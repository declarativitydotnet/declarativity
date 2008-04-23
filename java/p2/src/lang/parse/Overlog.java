package lang.parse;

import java.io.File;
import java.io.IOException;
import java.io.Reader;

import core.Catalog;

import xtc.Constants;
import xtc.parser.ParseException;
import xtc.tree.Node;
import xtc.tree.Visitor;
import xtc.type.TypePrinter;
import xtc.util.SymbolTable;
import xtc.util.Tool;


/**
 * The driver for processesing the Overlog language.
 */
public class Overlog extends Tool {
	
	private TypeChecker typeChecker;

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
    return (Node)parser.value(parser.pProgram(0));
  }

  public void process(Node node) {
	  // Perform type checking.
	  typeChecker.analyze(node);
	  // Print the symbol table.
	  if (runtime.test("printSymbolTable")) {
		  // Save the registered visitor.
		  Visitor visitor = runtime.console().visitor();
		  // Note that the type printer's constructor registers the just
		  // created printer with the console.
		  new TypePrinter(runtime.console());
		  try {
			  typeChecker.table().root().dump(runtime.console());
		  } finally {
			  // Restore the previously registered visitor.
			  runtime.console().register(visitor);
		  }
		  runtime.console().flush();
	  }

	  // Print AST.
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
	  Catalog.initialize();
      new Overlog().run(args);
  }
}
