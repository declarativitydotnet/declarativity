## Catalog initialization should be a result of some metacompiled
## code,not hardwired here.  

# The driver for processesing the Overlog language.
require "lib/lang/parse/schema"
require 'lib/types/basic/tuple'
require 'lib/types/table/object_table'
require 'lib/lang/plan/watch_clause'
require 'lib/lang/plan/selection_term'
require 'lib/lang/plan/function'
require "lib/lang/plan/program"
require 'lib/lang/plan/planner'


class Compiler # in java, this is a subclass of xtc.util.Tool
  @@FILES =  ["/Users/joeh/devel/lincoln/ruby/lang/compile.olg", "/Users/joeh/devel/lincoln/ruby/lang/stratachecker.olg"]

  # @@compiler   = CompilerTable.new
  # # @@programs   = ProgramTable.new
  # @@rule       = RuleTable.new
  # @@watch      = WatchTable.new
  # @@fact       = FactTable.new
  # @@predicate  = PredicateTable.new
  # @@tfunction  = Function::TableFunction.new
  # @@selection  = SelectionTable.new
  # @@assignment = AssignmentTable.new
  # @@preds = MyPredicateTable.new
  # @@terms = MyTermTable.new
  # @@pexpr = MyPrimaryExpressionTable.new
  # @@expr = MyExpressionTable.new
  # @@facts = MyFactTable.new
  # @@tables = MyTableTable.new
  # @@columns = MyColumnTable.new
  # @@indices = MyIndexTable.new
  # @@programs = MyProgramTable.new
  # @@rules = MyRuleTable.new
  # @@selects = MySelectionTable.new
  # @@assigns = MyAssignmentTable.new


  # Create a new driver for Overlog.
  def initialize(owner, file)
    @owner = owner
    @file = file
    utterance = ''
    File.open(file) do |f|
      f.each_line { |l| utterance << l }
    end
    # not quiet sure why, but this call here to "init_catalog" seems to be
    # needed.  Requires more investigation, would be nice to chuck this (and 
    # the init_catalog method)
    Compiler.init_catalog
    planner = OverlogPlanner.new(utterance,@@rules,@@terms,@@preds,@@pexpr,@@expr,@@facts,@@tables,@@columns,@@indices,@@programs,@@assigns,@@selects)
    planner.plan
    @program = planner.program		

    #    typeChecker = TypeChecker.new(@runtime, @program)
    # This is an XTC-ism
    # args = ["-no-exit", "-silent", file]
    # run(args)

    # if (runtime.errorCount > 0) then
    #    @program.definitions.each { |table| Table.drop(table.name) }
    #  end
  end	

  def Compiler.init_catalog
    @@compiler   = CompilerTable.new unless defined? @@compiler
#    @@programs   = ProgramTable.new unless defined? @@programs
    @@rule       = RuleTable.new unless defined? @@rule
    @@watch      = WatchTable.new unless defined? @@watch
    @@fact       = FactTable.new unless defined? @@fact
    @@predicate  = PredicateTable.new unless defined? @@predicate
    @@tfunction  = Function::TableFunction.new unless defined? @@tfunction
    @@selection  = SelectionTable.new unless defined? @@selection
    @@assignment = AssignmentTable.new unless defined? @@assignment
    @@preds = MyPredicateTable.new unless defined? @@preds
    @@terms = MyTermTable.new unless defined? @@terms
    @@pexpr = MyPrimaryExpressionTable.new unless defined? @@pexpr
    @@expr = MyExpressionTable.new unless defined? @@expr
    @@facts = MyFactTable.new unless defined? @@facts
    @@tables = MyTableTable.new unless defined? @@tables
    @@columns = MyColumnTable.new unless defined? @@columns
    @@indices = MyIndexTable.new unless defined? @@indices
    @@programs = MyProgramTable.new unless defined? @@programs
    @@rules = MyRuleTable.new unless defined? @@rules
    @@selects = MySelectionTable.new unless defined? @@selects
    @@assigns = MyAssignmentTable.new unless defined? @@assigns
  end

  attr_reader :program, :watch
  def Compiler.files
    @@FILES
  end
  
  def Compiler.compiler
    @@compiler
  end
  def Compiler.rule
    @@rule
  end
  def Compiler.watch
    @@watch
  end
  def Compiler.fact
    @@fact
  end
  def Compiler.predicate
    @@predicate
  end
  def Compiler.tfunction
    @@tfunction
  end
  def Compiler.selection
    @@selection
  end
  def Compiler.assignment
    @@assignment
  end
  def Compiler.preds
    @@preds
  end
  def Compiler.terms
    @@terms
  end
  def Compiler.pexpr
    @@pexpr
  end
  def Compiler.expr
    @@expr
  end
  def Compiler.facts
    @@facts
  end
  def Compiler.tables
    @@tables
  end
  def Compiler.columns
    @@columns
  end
  def Compiler.indices
    @@indices
  end
  def Compiler.programs
    @@programs
  end
  def Compiler.rules
    @@rules
  end
  def Compiler.selects
    @@selects
  end
  def Compiler.assigns
    @@assigns
  end



  def getName 
    return "OverLog Compiler"
  end

  def getCopy 
    return Constants.FULL_COPY
  end

  def parse(input, file)
    parser = new Parser(input, file.to_s, file.length)
    return parser.value(parser.program(0))
  end

  def process(node)
    name = node.getString(0)

    # Perform type checking.
    # runtime.console.format(node).pln.flush
    @program = Program.new(name, owner)
    @typeChecker = TypeChecker.new(@runtime, @program)
    @typeChecker.prepare

    # First evaluate all import statements.
    node.getNode(1).<Node>getList(0).each do |clause|
      if (clause.getName == "Import") then
        typeChecker.analyze(clause)
        return if (runtime.errorCount > 0)
      end
    end

    # Next evaluate all table and event declarations.
    node.getNode(1).<Node>getList(0).each do |clause|
      if (clause.getName.equals("Table")) then
        typeChecker.analyze(clause)
        return if (runtime.errorCount > 0)
      elsif (clause.getName.equals("Event")) then
        typeChecker.analyze(clause)
        return if (runtime.errorCount > 0)
      end
    end

    # All programs define a local periodic event table
    periodic = TableName.new(program.name, "periodic")
    program.definition(EventTable.new(periodic, TypeList.new(Periodic::SCHEMA)))

    # Evaluate all other clauses.
    node.getNode(1).<Node>getList(0) do |clause|
      if (clause.getName == "Rule" or clause.getName == "Fact" or clause.getName == "Watch") then
        typeChecker.analyze(clause)
        return if (runtime.errorCount > 0)
        if (clause.getName == "Watch") then
          watches = clause.getProperty(Constants.TYPE)
          watches.each { |w| w.set(@program.name) }
        else
          c = clause.getProperty(Constants.TYPE)
          c.set(@program.name)
        end
      end
    end
  end
end
