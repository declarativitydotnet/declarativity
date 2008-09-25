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
  ##@@FILES =  ["/Users/joeh/devel/lincoln/ruby/lib/lang/compile.olg", "/Users/joeh/devel/lincoln/ruby/lib/lang/stratachecker.olg"]

  @@FILES =  ["lib/lang/compiler.olg", "lib/lang/stratachecker.olg"]

  # Create a new driver for Overlog.
  def initialize(owner, file)
    @owner = owner
    @file = file
    utterance = ''
    File.open(file) do |f|
      f.each_line { |l| utterance << l }
    end
    # Since this is a new compiler, we need to wipe out any old compiler catalog stuff
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
      table[0] = table[0..0].downcase
      table_name = TableName.new(CompilerCatalogTable::COMPILERSCOPE,table)
      t = Table.find_table(table_name)
      # if table.downcase == "rule" and (not t.nil?)
      #   require 'ruby-debug'; debugger
      # end
     unless t.nil?
        t.drop 
        unless $catalog.primary.lookup_vals(table_name).size == 0
          require 'ruby-debug; debugger'
          raise 'table {table_name} still lurking in catalog after delete'
        end
        raise "drop failed" unless Table.find_table(table_name).nil?
      end
      varname = camelize(table)
      str = "@@"+ varname + " = " + c.name + ".new" 
#      print "\t"+str+"\n"
	    eval(str)
	    @@syms << eval(":" + varname)
    end
    cattr_reader(@@syms)
  end

  attr_reader :program
  
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
      if clause.getName == "Import"
        typeChecker.analyze(clause)
        return if (runtime.errorCount > 0)
      end
    end

    # Next evaluate all table and event declarations.
    node.getNode(1).<Node>getList(0).each do |clause|
      if clause.getName.equals("Table")
        typeChecker.analyze(clause)
        return if runtime.errorCount > 0
      elsif clause.getName.equals("Event")
        typeChecker.analyze(clause)
        return if runtime.errorCount > 0
      end
    end

    # All programs define a local periodic event table
    periodic = TableName.new(program.name, "periodic")
    program.definition(EventTable.new(periodic, TypeList.new(Periodic::SCHEMA)))

    # Evaluate all other clauses.
    node.getNode(1).<Node>getList(0) do |clause|
      if (clause.getName == "Rule" or clause.getName == "Fact" or clause.getName == "Watch")
        typeChecker.analyze(clause)
        return if runtime.errorCount > 0
        if clause.getName == "Watch"
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
