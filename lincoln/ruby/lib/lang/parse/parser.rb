require "rubygems"
require "treetop"
require "core.rb"
#require "olg.rb"

require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'

require "Treewalker.rb"
require 'local_tw.rb'
require 'output.rb'

verbose = 'n'

prog = ''
while line = STDIN.gets
	prog = prog + line
end

parser = OverlogParser.new
result = parser.parse(prog)
if result
  #puts 'success'
else
  puts 'failure'
	raise RuntimeError.new(parser.failure_reason)
	exit
end


#puts result.inspect


#ve = VisitExpression.new


sky = Treewalker.new(result)

vg = VisitGeneric.new

sky.add_handler("Word",vg,1)
sky.add_handler("Location",vg,1)
sky.add_handler("Watch",vg,1)
#sky.add_handler("Expression",vg,1)
sky.add_handler("PrimaryExpression",vg,1)
sky.add_handler("Predicate",VisitPredicate.new,1)
sky.add_handler("Fact",VisitFact.new,1)
sky.add_handler("Definition",VisitTable.new,1)
sky.add_handler("TableName",vg,1)
sky.add_handler("Schema",vg,1)
sky.add_handler("Rule",VisitRule.new,1)
sky.add_handler("Selection",VisitSelection.new,1)
sky.add_handler("Assignment",VisitAssignment.new, 1)

sky.add_handler("Variable",VisitVariable.new,1)
sky.add_handler("Constant",VisitConstant.new,1)


sky.add_handler("Aggregate",vg,1)
sky.add_handler("Name",vg,1)
sky.add_handler("AggregateVariable",vg,1)

#sky.add_handler("Arguments",vg,1)

sky.add_handler("Periodic",vg,1)
sky.add_handler("Table",VisitTable.new,1)
sky.add_handler("Type",VisitColumn.new,1)
sky.add_handler("Keys",VisitIndex.new,1)


init_output("foo")
sky.walk(verbose)
