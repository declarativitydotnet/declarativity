require "test/unit"

require 'local_tw.rb'
require 'Treewalker.rb'
#require 'termtab.rb'

require 'schema.rb'

require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require "lib/types/operator/scan_join"

class TestParse < Test::Unit::TestCase
	def default_test

		#test_fact
		#test_preds


		exit

	end

	def test_join1
		prep("program foo;\nfoo(A,B) :- bar(A,B);\n")
		
		schema = @preds.schema_of
		pred = Predicate.new(false,@preds.name, @preds, schema.variables)
		pred.set("myprog", "r3", 1) 
		sj = ScanJoin.new(pred, schema)	
	end

	def test_preds
		prep("program foo;\nfoo(A,B) :- bar(A,B);\n")


		@preds.tuples.each do |t|
			name = t.values[2]
			if (name.eql?("bar")) then
				assert_equal(t.to_s,"<13, 12, bar, 1, null>")
			elsif (name.eql?("foo")) then
				assert_equal(t.to_s,"<4, 3, foo, 0, null>")
			else
				assert_error("buh?")
			end
		end

	end

	def rei
		@preds = PredicateTable.new
		@terms = TermTable.new
		@pexpr = PrimaryExpressionTable.new
		@expr = ExpressionTable.new
		@facts = FactTable.new
	end
	
	def test_fact
		prep("program foo;\npath(\"1\",\"2\");\n")
		tups = Array.new
		@terms.tuples.each do |t|
			tups << t
		end	
		assert_equal(tups.size,1)
		assert_equal(tups[0].to_s,"<2, 0, 0, path>")

		
		tups = Array.new
		@facts.tuples.each do |t|
			print "FACT: "+t.to_s+"\n"
			tups << t
		end	
		assert_equal(tups.size,1)
		assert_equal(tups[0].to_s,"<3, 2, path>")
	end

	def prep(utterance)
		rei
		compiler = OverlogCompiler.new(nil,@terms,@preds,@pexpr,@expr,@facts)
		compiler.parse(utterance)
		compiler.analyze
	end
	
end
