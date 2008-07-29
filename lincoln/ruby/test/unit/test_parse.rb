require "test/unit"

require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'

require 'lib/lang/parse/schema.rb'

require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
#require 'lib/lang/compiler.rb'
#require 'lib/lang/plan/program.rb'
#require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"


def predoftable(table)
        schema = table.schema_of
        p = Predicate.new(false,table.name,table,schema.variables)
	p.set("global","r1",1)
	return p
end


class TestParse < Test::Unit::TestCase
  def test_default
  end

$catalog = Catalog.new
$index = IndexTable.new
	def test_default
		test_program
	end
	def test_prog
		prep("program foo;\nfoo(A,B) :- bar(A,B);\n")

		# materializations... later
	
		# 
		print "OK THEN\n"
		puts @programs
		puts @rules

		p_rule = predoftable(@rules)
	
		sj = ScanJoin.new(p_rule,@programs.schema_of)
		ts = TupleSet.new("prog",*@programs.tuples)

		res = sj.evaluate(ts)
		
		p_term = predoftable(@terms)
		sterm = ScanJoin.new(p_term,res.tups[0].schema)
		resterm = sterm.evaluate(res)

		p_expr = predoftable(@expr)
		sexpr = ScanJoin.new(p_expr,resterm.tups[0].schema)
		resexpr = sexpr.evaluate(resterm)

		resexpr.tups.each do |t|
			print "TUP: "+t.to_s+"\n"
			puts t.inspect
			puts t.schema

			print "foo\n"
		end
		
		
	end
	def test_join1
		prep("program foo;\nfoo(A,B) :- bar(A,B);\n")
		
		# set up schema table's predicate
		#require 'ruby-debug'; debugger
		term_schema = @terms.schema_of
		print "terms.name is "+@terms.name.to_s+"\n"
		term_pred  = Predicate.new(false,@terms.name,@terms,term_schema.variables)
		term_pred.set("global", "r3", 1)
		
		sj = ScanJoin.new(term_pred, @preds.schema_of)	
		ts = TupleSet.new("pred", *@preds.tuples)
		res = sj.evaluate(ts)

		#res.tups.each do |t|
		#	print "TUP: "+t.to_s+"\n"
		#end
		assert_equal(2, res.tups.length)
	end

	def test_program
		prep("program foo;\npath(A,B,_) :- path(B,Z,C);\n")
		assert_equal(1,@programs.cardinality)
	end
		
	def test_deletion
		prep("program foo;\ndelete path(A,B,_) :- path(B,Z,C);\n")
	end

	def test_aggregation
		prep("program foo;\nshortestPath(A,B,min<C>) :- path(A,B,C);\n")
		#nnputs @expr
		#puts @pexpr
			
	end


	def test_materialize
		prep("program foo;\ndefine(path,keys(0,1),{String,String,Integer});\npath(\"a\",\"b\");\n")
		
		# we should have a table entry
		assert_equal(@tables.cardinality,1)		
		# with 3 columns
		assert_equal(@columns.cardinality,3)
		# and 2 index entries
		assert_equal(@indices.cardinality,2)
	end

	def test_expr
		prep("program foo;\nfoo(A,B,B + 1) :- bar(A,B);\n")

		#puts @preds
		#puts @expr
		#puts @pexpr
		# there are 2 expression in predicate foo, but 3 primary expressions
		assert_equal(@pexpr.cardinality,6)
		assert_equal(@expr.cardinality,5)
		foundconst = 0
		@pexpr.tuples.each do |t|
			# don't forget to fix the quotes
			if t.values[3].eql?("\"1\"") then
				# we have an integer constant here. 
				assert_equal(t.values[4],"const")
				foundconst = 1
			end
		end
		# fallthrough
		assert_equal(foundconst,1)
	
	end

	def test_arbitrary_expr
		prep("program foo;\nfoo(A,B,(B + 1) / (A-B*A) / 2) :- bar(A,B);\n")
		#puts @expr
		#puts @pexpr
		# there are still only 3 arguments to foo and 2 to bar.
		#assert_equal(@expr.cardinality,5)
		# however, we are dealing with 10 primaryexpressions
		#assert_equal(@pexpr.cardinality,10)

	end

	def test_preds
		prep("program foo;\nfoo(A,B) :- bar(A,B);\n")

		
		@preds.tuples.each do |t|
			name = t.values[2]
			if (name.eql?("bar")) then
				assert_equal(t.to_s,"<12, 11, bar, 1>")
			elsif (name.eql?("foo")) then
				assert_equal(t.to_s,"<4, 3, foo, 0>")
			else
				assert_error("buh?")
			end
		end
		# there should be a 1-1 between primaryexpressions and expressions.
		assert_equal(@pexpr.cardinality,@expr.cardinality)
		# (it should be 4)
		assert_equal(@pexpr.cardinality,4)
	end

	def rei
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
	end
	
	def test_fact
		prep("program foo;\npath(\"1\",\"2\");\n")
		tups = Array.new
		@programs.tuples.each do |t|
			tups << t
		end	

		puts @programs
		puts @facts

		print "ALLO\n"
		assert_equal(tups.size,1)
		assert_equal(tups[0].to_s,"<1, nil, foo>")

		
		tups = Array.new
		@facts.tuples.each do |t|
			print "FACT: "+t.to_s+"\n"
			tups << t
		end	
		assert_equal(tups.size,1)
		assert_equal(tups[0].to_s,"<3, 1, path>")
	end

	def prep(utterance)
		rei
		compiler = OverlogCompiler.new(@rules,@terms,@preds,@pexpr,@expr,@facts,@tables,@columns,@indices,@programs)
		compiler.verbose = 'y'
		compiler.parse(utterance)
		compiler.analyze
	end
	
end
