# The driver for processesing the Overlog language.
require "lib/lang/parse/schema"
require 'lib/types/basic/tuple'
require 'lib/types/table/object_table'
require 'lib/lang/plan/watch_clause'
require 'lib/lang/plan/selection_term'
require 'lib/lang/plan/function'
require "lib/lang/plan/program"

class Compiler # in java, this is a subclass of xtc.util.Tool
  @@FILES =  ["/Users/joeh/devel/lincoln/ruby/lang/compile.olg", "/Users/joeh/devel/lincoln/ruby/lang/stratachecker.olg"]

  @@compiler   = CompilerTable.new
  @@programs   = ProgramTable.new
  @@rule       = RuleTable.new
  @@watch      = WatchTable.new
  @@fact       = FactTable.new
  @@predicate  = PredicateTable.new
  @@tfunction  = Function::TableFunction.new
  @@selection  = SelectionTable.new
  @@assignment = AssignmentTable.new

  # Create a new driver for Overlog.
  def initialize(owner, file)
    @owner = owner
    @file = file
#    typeChecker = TypeChecker.new(@runtime, @program)
    # This is an XTC-ism
    # args = ["-no-exit", "-silent", file]
    # run(args)

   # if (runtime.errorCount > 0) then
  #    @program.definitions.each { |table| Table.drop(table.name) }
  #  end
  end	
  
  def Compiler.rule
    @@rule
  end

  attr_reader :program, :watch

  def getName 
    return "OverLog Compiler"
  end

  def getCopy 
    return Constants.FULL_COPY
  end

  def init 
    super.init
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
