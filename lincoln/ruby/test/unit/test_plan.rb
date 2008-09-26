require "test/unit"

require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'

require 'lib/lang/parse/schema.rb'

require 'lib/lang/plan/planner.rb'
require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
#require 'lib/lang/@rb'
require 'lib/lang/plan/program.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"
require "lib/lang/plan/arbitrary_expression.rb"

require 'lib/lang/parse/procedural.rb'


class TestPlan < Test::Unit::TestCase
	
	def gen_link_tuples(prog)
	  # we need to manually construct a schema spec; we can't infer it from link (materialized by the overlog above) 
		# because link is empty, and schema belong to tuples, not tables (not that link's fields have no names)
		t1 = Tuple.new("N1","N2",10,"first")
		t2 = Tuple.new("N2","N3",5,"second")

		# ... from test_program
		v1 = Variable.new("From", String, 0)
		v2 = Variable.new("To", String, 1)
		v3 = Variable.new("Cost", Float, 2)
		v4 = Variable.new("Annotation", String, 3)

		schema1 = Schema.new("schema1", [v1,v2,v3,v4])
		t1.schema = schema1
		t2.schema = schema1
		tn = TableName.new(prog, "link")
		assert_equal(1,1)
		return tn, TupleSet.new(tn, t1, t2)		
	end
	
	def prep(utterance)

		rei
		Compiler.init_catalog
		planner = OverlogPlanner.new(utterance)
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
		@tfuncs = MyTableFunctionTable.new
	end
	
	def test_notin
    $catalog=nil; $index=nil
    sys=System.new; sys.init
		utterance = "program path;

"

		
	end
	def test_watch
		$catalog=nil; $index=nil
    sys=System.new; sys.init
                prep("program foo;
                        define(arg,keys(0,1),{String,String,Integer});
                        define(link,keys(0,1),{String,String,Integer});
                        //watch(arg,id);
                        arg(A,B,C) :- link(A,B), C := 1;
")
		
	end
	def test_materialization
		$catalog=nil; $index=nil
    sys=System.new; sys.init
		utterance = "program path;
				define(link,keys(0,1),{String,String,Integer,String});
                                path(A,B,C,D) :- link(A,B,C,D);
"
		prog = prep(utterance)
		assert_equal("0,1",prog.definitions[0].key.attributes.join(","))
		assert_equal("String,String,Integer,String",prog.definitions[0].types.join(","))

	end

        def test_materialization2
                # DRY?  yes yes I know.
                # but we're illustrating equivalence.  the assertions should
                # be encapsulated in a function, but in this test framework
                # all subroutines are automatically executed!

                sys = System.new
                $catalog=nil
                $index=nil
                sys.init
                utterance = "program path;
                                table link(
                                        +From String,
                                        +To String,
                                        Cost Integer,
                                        Annotation String
                                );
                                path(A,B,C,D) :- link(A,B,C,D);

"
                prog = prep(utterance)
                assert_equal("0,1",prog.definitions[0].key.attributes.join(","))
                assert_equal("String,String,Integer,String",prog.definitions[0].types.join(","))
        end


	def test_oldprog1
		$catalog=nil; $index=nil
    sys=System.new; sys.init
		utterance = "program path;
				define(link,keys(0,1),{String,String,Integer,String});
				path(From,To,Cost) :- link(From,To,Cost,Annotation);
"
		prog = prep(utterance)

    tn, ts = gen_link_tuples("path")
		result = prog.get_queries(tn)[0].evaluate(ts)
		assert_equal(result.tups.length, 2)
		assert_equal(result.tups[0].values, ["N1","N2",10])
		assert_equal(result.tups[1].values, ["N2","N3",5])
	end

	def test_prog
		$catalog=nil; $index=nil
    sys=System.new; sys.init

		utterance = "program foo;\ndefine(path,keys(0,1),{String,String});
		    define(link,keys(0,1),{String,String});
		    path(A,B) :- link(A,B);"
		cooked_program = prep(utterance)

		tn  = TableName.new("foo","link")
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
    $catalog=nil; $index=nil
    sys=System.new; sys.init

		#P2
		utterance = "program foo;
				define(path,keys(0,1),{String,String,Integer});
				define(link,keys(0,1),{String,String});
				path(A,B,Cost) :- link(A,B),Cost := 1;
				path(A,B,Cost) :- link(A,Z), path(Z,B,C), Cost := C + 1;
				path(\"there\", \"elsewhere\", 1);"
		cooked_program = prep(utterance)

		#puts @assigns

		tn  = TableName.new("foo","link")
		#print "get queries:\n"
		queries = cooked_program.get_queries(tn)

		# what do we expect?  just the 1 delta-rewritten version of this rule.
		#print "query count = "+queries.length.to_s+"\n"
		#assert_equal(1,queries.length)

		# create some driver data
		tuple = Tuple.new("here","there")
		ts = TupleSet.new(tn,tuple)

		#print "queries[0] = #{queries[0]}\n"

		result = queries[0].evaluate(ts)
		result2 = queries[1].evaluate(ts)
		
		# we should get this right back.
		#assert_equal(["here", "there", 1], result.tups[0].values)
    # p.a. -- don't forget to replace this
		##assert_equal(["here", "elsewhere", 2], result2.tups[0].values)
		
		result.each do |t|
		end
		#puts cooked_program.inspect
	end
	
  def test_agg
    $catalog=nil
    sys = System.new
    sys.init
    utterance = "program agg_test;
    define(link,keys(0,1),{String,String,Integer,String});
    min_cost(From,To,avg<Cost>) :- link(From,To,Cost,Annotation);
    counter(From,To,count<Cost>) :- link(From,To,Cost,Annotation);"
    prog = prep(utterance)
    tn, ts = gen_link_tuples("agg_test")
    result = prog.get_queries(tn)[0].evaluate(ts)
		assert_equal(result.tups.length, 2)
		result.tups.each do |t|
			assert(t.values == ["N1","N2",10.0] || t.values == ["N2","N3",7.5])
		end
		#assert_equal(result.tups[0].values, ["N1","N2",10.0])
		#assert_equal(result.tups[1].values, ["N2","N3",7.5])
    result = prog.get_queries(tn)[1].evaluate(ts)
		assert_equal(result.tups.length, 2)
		result.tups.each do |t|
			assert(t.values == ["N1","N2",1] || t.values == ["N2","N3",2])
		end
  end

  def test_event
    $catalog=nil
    $index=nil
    sys = System.new
    sys.init
    utterance = "program test_event;
				define(link,keys(0,1),{String,String,Integer,String});
				define(delta,keys(0,1),{String,String,Integer,String});
				define(epsilon,keys(0,1),{String,String,Integer,String});
				delta(A,B,C,D) :- link#insert(A,B,C,D);
				epsilon(A,B,C,D) :- link#delete(A,B,C,D);				
				"
		prog = prep(utterance)
		tn, ts = gen_link_tuples("test_event")
		
		result = prog.get_queries(tn)[0].evaluate(ts)
		assert_equal(result.tups.length, 2)
		result.tups.each do |t|
			assert(t.values == ["N1","N2",10,"first"] || t.values == ["N2","N3",5,"second"])
		end		

		
		# Should be 0 tuples in epsilon!  Doesn't currently work.

		#prog.get_queries(tn).each do |q|
		#	puts q.inspect
		#end

		r2 = prog.get_queries(tn)[1].evaluate(ts)
		#assert_equal(0, r2.tups.length)
	end

  def test_require
    $catalog=nil
    $index=nil
    sys = System.new
    sys.init
    utterance = "program reqtest;
      require \"webrick\";
      define(link,keys(0,1),{String, String, Integer, Tuple});
      define(path,keys(0,1),{String, String, Integer, Tuple});
      path(A,B,C,T) :- link(A, B, C, T);
    "
    prog = prep(utterance)
    tn, ts = gen_link_tuples("reqtest")

		result = prog.get_queries(tn)[0].evaluate(ts)
		assert_equal(result.tups.length, 2)
		result.tups.each do |t|
			assert( (t.values == ["N1","N2",10,"first"]) || (t.values == ["N2","N3",5,"second"])) 
		end
  end

	def test_facts
		$catalog=nil; $index=nil
    sys=System.new; sys.init
		utterance = "program path;
				define(link,keys(0,1),{String,String,Integer,String});
				link(\"A\",\"B\",3,\"link 1\");
				link(\"B\",\"C\",2,\"link 2\");
				path(From,To,Cost) :- link(From,To,Cost,Annotation);
"
		prog = prep(utterance)
		p = prog.plan

		link = Table.find_table("path::link")
	
		link.tuples.tups.each do |t|
			assert(t.to_s == "<A, B, 3, link 1>" || t.to_s == "<B, C, 2, link 2>")
		end		
		
	end


	
	def test_assign
	 	$catalog=nil; $index=nil
    sys=System.new; sys.init
		utterance = "program path;
		      define(link,keys(0,1),{String,String,Integer,String});
	        echo(X) :- link(F,T,C,A), B := Array.new(), X:= B.push(2);"
	  prog = prep(utterance)
    tn, ts = gen_link_tuples("path")

    assert(!prog.get_queries(tn).nil?)
		result = prog.get_queries(tn)[0].evaluate(ts)
		assert_equal(result.tups.length, 2)
		assert_equal(result.tups[0].values, [[2]])
    
  end
end
