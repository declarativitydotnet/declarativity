require "test/unit"

require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'

require 'lib/lang/parse/schema.rb'

require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
require 'lib/lang/compiler'
#require 'lib/lang/plan/program.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'

require 'lib/types/table/index_table.rb'

require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"

class TestParse < Test::Unit::TestCase
  # 
  # $catalog = Catalog.new
  # $index = IndexTable.new
  # def test_join1
  #   prep("program foo;\nfoo(A,B) :- bar(A,B);\n")
  # 
  #   # set up schema table's predicate
  #   #require 'ruby-debug'; debugger
  #   term_schema = @terms.schema_of
  #   # print "terms.name is "+@terms.name.to_s+"\n"
  #   term_pred  = Predicate.new(false,@terms.name,@terms,term_schema.variables)
  #   term_pred.set("global", "r3", 1)
  # 
  #   sj = ScanJoin.new(term_pred, @preds.schema_of)  
  #   ts = TupleSet.new("pred", *@preds.tuples)
  #   require 'ruby-debug'; debugger
  #   res = sj.evaluate(ts)
  # 
  #   # res.tups.each do |t|
  #   #   print "TUP: "+t.to_s+"\n"
  #   # end
  #   assert_equal(2, res.tups.length)
  # end
  # 
  # def test_deletion
  #   prep("program foo;\ndelete path(A,B,_) :- path(B,Z,C);\n")
  # end
  # 
  # def test_aggregation
  #   require 'ruby-debug'; debugger
  #   prep("program foo;\nshortestPath(A,B,min<C>) :- path(A,B,C);\n")
  #  # puts @expr
  #  #puts @pexpr
  # 
  # end
  # 
  # 
  # def test_materialize
  #   prep("program foo;\ndefine(path,keys(0,1),{String,String,Integer});\npath(\"a\",\"b\");\n")
  # 
  #   # we should have a table entry
  #   assert_equal(@tables.cardinality,1)   
  #   # with 3 columns
  #   assert_equal(@columns.cardinality,3)
  #   # and 2 index entries
  #   assert_equal(@indices.cardinality,2)
  # end
  # 
  # def test_expr
  #   prep("program foo;\nfoo(A,B,B + 1) :- bar(A,B);\n")
  # 
  #   #puts @preds
  #   #puts @expr
  #   #puts @pexpr
  #   # there are 2 expression in predicate foo, but 3 primary expressions
  #   assert_equal(@pexpr.cardinality,6)
  #   assert_equal(@expr.cardinality,5)
  #   foundconst = 0
  #   @pexpr.tuples.each do |t|
  #     # don't forget to fix the quotes
  #     if t.values[3].eql?("\"1\"") then
  #       # we have an integer constant here. 
  #       assert_equal(t.values[4],"const")
  #       foundconst = 1
  #     end
  #   end
  #   # fallthrough
  #   assert_equal(foundconst,1)
  # 
  # end
  # 
  # def test_arbitrary_expr
  #   prep("program foo;\nfoo(A,B,(B + 1) / (A-B*A) / 2) :- bar(A,B);\n")
  #   # puts @expr
  #   # puts @pexpr
  #   # there are still only 3 arguments to foo and 2 to bar.
  #   #assert_equal(@expr.cardinality,5)
  #   # however, we are dealing with 10 primaryexpressions
  #   #assert_equal(@pexpr.cardinality,10)
  # 
  # end
  # 
  # def test_preds
  #   prep("program foo;\nfoo(A,B) :- bar(A,B);\n")
  # 
  # 
  #   @preds.tuples.each do |t|
  #     name = t.values[2]
  #     if (name.eql?("bar")) then
  #       assert_equal(t.to_s,"<12, 11, bar, 1, nil>")
  #     elsif (name.eql?("foo")) then
  #       assert_equal(t.to_s,"<4, 3, foo, 0, nil>")
  #     else
  #       assert_error("buh?")
  #     end
  #   end
  #   # there should be a 1-1 between primaryexpressions and expressions.
  #   assert_equal(@pexpr.cardinality,@expr.cardinality)
  #   # (it should be 4)
  #   assert_equal(@pexpr.cardinality,4)
  # end
  # 
  # def rei
  #   $catalog = Catalog.new
  #   $index = IndexTable.new
  # 
  #   @preds = PredicateTable.new
  #   @terms = TermTable.new
  #   @pexpr = PrimaryExpressionTable.new
  #   @expr = ExpressionTable.new
  #   @facts = FactTable.new
  #   @tables = TableTable.new
  #   @columns = ColumnTable.new
  #   @indices = MyIndexTable.new
  # end
  # 
  # def test_fact
  #   prep("program foo;\npath(\"1\",\"2\");\n")
  #   tups = Array.new
  #   @terms.tuples.each do |t|
  #     tups << t
  #   end 
  #   assert_equal(tups.size,1)
  #   assert_equal(tups[0].to_s,"<2, 0, 0, path>")
  # 
  # 
  #   tups = Array.new
  #   @facts.tuples.each do |t|
  #     # print "FACT: "+t.to_s+"\n"
  #     tups << t
  #   end 
  #   assert_equal(tups.size,1)
  #   assert_equal(tups[0].to_s,"<3, 2, path>")
  # end
  # 
  # def prep(utterance)
  #   rei
  #   compiler = OverlogCompiler.new(nil,@terms,@preds,@pexpr,@expr,@facts,@tables,@columns,@indices)
  #   compiler.verbose = 'y'
  #   compiler.parse(utterance)
  #   compiler.analyze
  # end

end
