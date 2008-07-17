require "rubygems"
require "treetop"
require "core.rb"
#require "olg.rb"

require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'

require "Treewalker.rb"

require 'local_tw.rb'

require 'output.rb'

prog = ''
while line = STDIN.gets
	prog = prog + line
end

parser = OverlogParser.new
result = parser.parse(prog)
if result
  puts 'success'
else
  puts 'failure'
	raise RuntimeError.new(parser.failure_reason)
	exit
end


#puts result.inspect


#ve = VisitExpression.new
vg = VisitGeneric.new
vf = VisitFact.new
vp = VisitPredicate.new
vv = VisitVariable.new
vc = VisitConstant.new

vs = VisitSelection.new

sky = Treewalker.new(result)



sky.add_handler("Word",vg,1)

sky.add_handler("Location",vg,1)

sky.add_handler("Watch",vg,1)
sky.add_handler("Expression",vg,1)
sky.add_handler("Predicate",vp,1)

sky.add_handler("Fact",vf,1)
sky.add_handler("Definition",vg,1)

sky.add_handler("TableName",vg,1)

sky.add_handler("Keys",vg,1)
sky.add_handler("Schema",vg,1)

sky.add_handler("RuleBody",vg,1)

sky.add_handler("Selection",vs,1)
sky.add_handler("Assignment",vg,1)

sky.add_handler("Variable",vv,1)
sky.add_handler("Constant",vc,1)


sky.add_handler("Aggregate",vg,1)
sky.add_handler("Name",vg,1)
sky.add_handler("AggregateVariable",vg,1)

sky.add_handler("Arguments",vg,1)

sky.add_handler("Periodic",vg,1)




sky.walk()
