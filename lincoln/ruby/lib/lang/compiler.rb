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
require 'lib/lang/plan/object_from_catalog'


class Compiler # in java, this is a subclass of xtc.util.Tool
  extend ObjectFromCatalog
  @@FILES =  ["/Users/joeh/devel/lincoln/ruby/lang/compile.olg", "/Users/joeh/devel/lincoln/ruby/lang/stratachecker.olg"]

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
    planner = OverlogPlanner.new(utterance)
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

  # taken from Rails' Class definition
  def Compiler.cattr_reader(*syms)
    syms.flatten.each do |sym|
      next if sym.is_a?(Hash)
      str = "unless defined? @@"+sym.to_s+"\n@@"+sym.to_s+" = nil\nend\n\ndef self."+sym.to_s+"\n@@"+sym.to_s+"\nend\n\ndef "+sym.to_s+"\n@@"+sym.to_s+"\nend\n"
      class_eval(str, __FILE__, __LINE__)
    end
  end


  def Compiler.init_catalog
    @@syms = Array.new
    CompilerCatalogTable.classes.each do |c|
      table = c.name[0..(c.name.rindex("Table")-1)]
      table_name = TableName.new(Table::GLOBALSCOPE,table.downcase)
      t = Table.find_table(table_name)
      t.drop unless t.nil?
      varname = camelize(table)
      str = "@@"+ varname + " = " + c.name + ".new" 
#      print "\t"+str+"\n"
	    eval(str)
	    @@syms << eval(":" + varname)
    end
    cattr_reader(@@syms)
  end

  attr_reader :program, :watch
  
  def Compiler.files
    @@FILES
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
