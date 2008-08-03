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
	def test_oldprog1
		sys = System.new
		sys.init
		utterance = "program path;
				define(link,keys(0,1),{String,String,Integer,String});
				path(From,To,Cost) :- link(From,To,Cost,Annotation);
"

		prog = prep(utterance)

		# we need to manually construct a schema spec; we can't infer it from link (materialized by the overlog above) 
		# because link is empty, and schema belong to tuples, not tables (not that link's fields have no names)
		t1 = Tuple.new("1","2","10","first")
		t2 = Tuple.new("2","3","5","second")

		# ... from test_program
		v1 = Variable.new("From", Integer)
		v1.position = 0
		v2 = Variable.new("To", Integer)
		v2.position = 1
		v3 = Variable.new("Cost", Float)
		v3.position = 2
		v4 = Variable.new("Annotation", String)
		v4.position = 3   

		schema1 = Schema.new("schema1", [v1,v2,v3,v4])
		t1.schema = schema1
		t2.schema = schema1
		tn = TableName.new(nil, "link")
		tn = TableName.new(nil,"link")
		ts = TupleSet.new(tn, t1, t2)
		# ... ok, done with test_program

		result = prog.get_queries(tn)[0].evaluate(ts)
		assert_equal(result.tups.length, 2)
		assert_equal(result.tups[0].values, ["1","2","10"])
		assert_equal(result.tups[1].values, ["2","3","5"])
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

		#puts @assigns

		tn  = TableName.new(nil,"link")
		#print "get queries:\n"
		queries = cooked_program.get_queries(tn)

		# what do we expect?  just the 1 delta-rewritten version of this rule.
		#print "query count = "+queries.length.to_s+"\n"
		#assert_equal(1,queries.length)

		# create some real data
		tuple = Tuple.new("here","there")
		ts = TupleSet.new(tn,tuple)

		
		result = queries[0].evaluate(ts)
		#result2 = queries[1].evaluate(result)
		
		# we should get this right back.
		assert_equal("<here, there>",result.tups[0].to_s)
		
		result.each do |t|
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
