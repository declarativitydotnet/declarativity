require "test/unit"

require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'

require 'lib/lang/parse/schema.rb'

require 'lib/lang/plan/planner.rb'
require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
#require 'lib/lang/compiler.rb'
require 'lib/lang/plan/program.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"
require "lib/lang/plan/arbitrary_expression.rb"

require 'lib/lang/parse/procedural.rb'


class TestPlan < Test::Unit::TestCase
  def test_default
  end
	def test_prog
		sys = System.new
		sys.init

		utterance = "program foo;\ndefine(path,keys(0,1),{String,String});\ndefine(link,keys(0,1),{String,String});\npath(A,B) :- link(A,B);\n"
		cooked_program = prep(utterance)

		tn  = TableName.new(nil,"link")
		queries = cooked_program.get_queries(tn)

		# what do we expect?  just the 1 delta-rewritten version of this rule.
		#print "query count = "+queries.length.to_s+"\n"
		assert_equal(1,queries.length)

		# create some real data
		tuple = Tuple.new("here","there")
		ts = TupleSet.new(tn,tuple)
		result = queries[0].evaluate(ts)
		
		# we should get this right back.
		assert_equal("<here, there>",result.tups[0].to_s)
	end
	
	def test_prog2
		$catalog = nil
		sys = System.new
		sys.init

		#P2
		utterance = "program foo;
				define(path,keys(0,1),{String,String});
				define(link,keys(0,1),{String,String});
				path(A,B,Cost) :- link(A,B),Cost := 1;
				path(A,B,Cost) :- link(A,Z), path(Z,B,C), Cost := C + 1;
"
		cooked_program = prep(utterance)

		puts @assigns

		tn  = TableName.new(nil,"link")
		print "get queries:\n"
		queries = cooked_program.get_queries(tn)

		# what do we expect?  just the 1 delta-rewritten version of this rule.
		print "query count = "+queries.length.to_s+"\n"
		queries.each do  |q|
			print "query: "+q.to_s+"\n"
		end
		#assert_equal(1,queries.length)

		# create some real data
		tuple = Tuple.new("here","there")
		ts = TupleSet.new(tn,tuple)

		
		result = queries[0].evaluate(ts)
		#result2 = queries[1].evaluate(result)
		
		# we should get this right back.
		assert_equal("<here, there>",result.tups[0].to_s)
		
		result.each do |t|
			print "RESS::\n"
			puts t
		end
		#puts cooked_program.inspect

			
		
	end
	

	def prep(utterance)
		rei
		planner = OverlogPlanner.new(utterance,@rules,@terms,@preds,@pexpr,@expr,@facts,@tables,@columns,@indices,@programs,@assigns,@selects)
		planner.plan
		return planner.program
	end

	def rei
		@preds = MyPredicateTable.new
		@terms = MyTermTable.new
		@pexpr = MyPrimaryExpressionTable.new
		@expr = MyExpressionTable.new
		@facts = MyFactTable.new
		@tables = MyTableTable.new
		@columns = MyColumnTable.new
		@indices = MyIndexTable.new
		@programs = MyProgramTable.new
		@rules = MyRuleTable.new
		@selects = MySelectionTable.new
		@assigns = MyAssignmentTable.new
	end
	
	
	
end
