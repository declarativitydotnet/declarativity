require "test/unit"

require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'

require 'lib/lang/parse/schema.rb'

require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
#require 'lib/lang/compiler.rb'
require 'lib/lang/plan/program.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"


def predoftable(table)
    schema = table.schema_of
    p = Predicate.new(false,table.name,table,schema.variables)
    p.set("global","r1",1)
    return p
end

def lcl_pretty_print(tuple)
	0.upto(tuple.size - 1) do |i|
		print "\t[#{i}] #{tuple.schema.variables[i].name} = #{tuple.values[i]}\n"
	end
end


class TestParse < Test::Unit::TestCase
	def test_watch
		prep("program foo;
			define(arg,keys(0,1),{String,String,Integer});
			watch(arg,id);
			arg(A,B,C) :- link(A,B), C := 1;
")

	end
	def test_predarg
		prep("program foo;
			path(A,B) :- link(A,(B+C)/17,27*B);

")

		@expr.tuples.each do |e|
			# there are only three args to the largest predicate
			assert_operator(e.value("arg_pos"),:<,3)
		end
	end

	def test_join1
		prep("program foo;\nfoo(A,B) :- bar(A,B);\n")
		
		# set up schema table's predicate
		#require 'ruby-debug'; debugger
		term_schema = @terms.schema_of
		term_pred  = Predicate.new(false,@terms.name,@terms,term_schema.variables)
		term_pred.set("global", "r3", 1)
		
		sj = ScanJoin.new(term_pred, @preds.schema_of)	
		ts = TupleSet.new("pred", *@preds.tuples)
		res = sj.evaluate(ts)

		assert_equal(2, res.tups.length)
	end

	def test_program
		prep("program foo;\npath(A,B,_) :- path(B,Z,C);\n")
		assert_equal(1,@programs.cardinality)
		assert_equal(0,@rules.tuples.tups[0].value("delete"))

	end
		
	def test_deletion
		prep("program foo;\ndelete path(A,B,_) :- path(B,Z,C);\n")
		assert_equal(1,@rules.tuples.tups[0].value("delete"))

		
	end

	def test_aggregation
		prep("program foo;\nshortestPath(A,B,min<C>) :- path(A,B,C);\n")
			
	end


	def test_materialize
		prep("program foo;\ndefine(path,keys(0,1),{String,String,Integer});\npath(\"a\",\"b\",2);\n")
		
		# we should have a table entry
		assert_equal(@tables.cardinality,1)		
		# with 3 columns
		assert_equal(@columns.cardinality,3)
		# and 2 index entries
		assert_equal(@indices.cardinality,2)

		# AND, this statement should be equivalent
		prep("program foo;
			table path(
				+From String,
				+To String,
				Cost Integer
			);
			path(\"a\",\"b\",2);
")

		assert_equal(@tables.cardinality,1)		
		assert_equal(@columns.cardinality,3)
	end

	def test_expr
		prep("program foo;\nfoo(A,B,B + 1) :- bar(A,B);\n")

		# there are 2 expressions in program foo, but 6 primary expressions
		assert_equal(6,@pexpr.cardinality)
		assert_equal(6,@expr.cardinality)
		foundconst = false
		@pexpr.tuples.each do |t|
			# don't forget to fix the quotes
			if t.values[3].eql?("1")
				# we have an integer constant here. 
				assert_equal(t.values[4],"const")
				foundconst = true
			end
		end
		# fallthrough
		assert_equal(foundconst, true)
	end

	def test_arbitrary_expr
		prep("program foo;\nfoo(A,B,(B + 1) / (A-B*A) / 2) :- bar(A,B);\n")
		assert_equal(@expr.cardinality,12)
		# however, we are dealing with 10 primaryexpressions
		assert_equal(@pexpr.cardinality,10)
	end

  def test_named_rule
    prep("program foo;
          rule1
          foo(A,B) :- bar(A,B);
")
    ruletup = @rules.tuples.tups[0]
    assert_equal("rule1",ruletup.value("rulename"))
    
  end

	def test_preds
		prep("program foo;\nfoo(A,B) :- bar(A,B);\n")

		
		@preds.tuples.each do |t|
			name = t.values[3]
			if name.eql?("bar")
				##assert_equal(t.values,[12, 11, 1, "bar", nil])
				assert_equal(t.values,[10, 9, 1, "bar", nil])
			elsif name.eql?("foo")
				assert_equal(t.values,[4, 3, 0, "foo", nil])
			else
        puts @preds
				raise("buh?")
			end
		end
		# there should be a 1-1 between primaryexpressions and expressions.
		assert_equal(@pexpr.cardinality,@expr.cardinality)
		# (it should be 4)
		assert_equal(@pexpr.cardinality,4)
	end

	def rei
		$catalog = nil
		$index = nil
		$catalog = Catalog.new
		$index = IndexTable.new

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
	
	def test_fact
		prep("program foo;\npath(\"1\",\"2\");\n")
		tups = Array.new
		@programs.tuples.each do |t|
			tups << t
		end	

		assert_equal(tups.size,1)
		assert_equal(tups[0].to_s,"<1, nil, foo>")
		
		tups = Array.new
		@facts.tuples.each do |t|
			tups << t
		end	
		assert_equal(tups.size,1)
		assert_equal(tups[0].values[2],"path")		
	end

	def prep(utterance)
		rei
		compiler = OverlogCompiler.new(@rules,@terms,@preds,@pexpr,@expr,@facts,@tables,@columns,@indices,@programs,@assigns,@selects,@tfuncs)
		compiler.verbose = 'y'
		compiler.parse(utterance)
		compiler.analyze
	end
end
