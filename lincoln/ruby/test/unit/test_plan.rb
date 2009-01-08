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
    v1 = Variable.new("From", String, 0,nil)
    v2 = Variable.new("To", String, 1,nil)
    v3 = Variable.new("Cost", Float, 2,nil)
    v4 = Variable.new("Annotation", String, 3,nil)

    schema1 = Schema.new("schema1", [v1,v2,v3,v4])
    t1.schema = schema1
    t2.schema = schema1
    tn = TableName.new(prog, "link")
    assert_equal(1,1)
    return tn, TupleSet.new(tn, t1, t2)		
  end

  def prep(utterance)

    @runtime = rei
    planner = OverlogPlanner.new(@runtime, utterance)
    planner.plan
    return planner.program
  end

  def rei
    r = Runtime.new
    # @preds = MyPredicateTable.new(r)
    @preds = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myPredicate"))
    # @terms = MyTermTable.new(r)
    @terms = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myTerm"))
    # @pexpr = MyPrimaryExpressionTable.new(r)
    @pexpr = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myPrimaryExpression"))
    # @expr = MyExpressionTable.new(r)
    @expr = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myExpression"))
    # @facts = MyFactTable.new(r)
    @facts = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myFact"))    
    # @tables = MyTableTable.new(r)
    @tables = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myTable"))    
    # @columns = MyColumnTable.new(r)
    @columns = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myColumn"))    
    # @indices = MyIndexTable.new(r)
    @indices = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myIndex"))
    # @programs = MyProgramTable.new(r)
    @programs = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myProgram"))
    # @rules = MyRuleTable.new(r)
    @rules = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myRule"))
    # @selects = MySelectionTable.new(r)
    @selects = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"mySelection"))
    # @assigns = MyAssignmentTable.new(r)
    @assigns = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myAssignment"))
    # @tfuncs = MyTableFunctionTable.new(r)
    @tfuncs = r.catalog.table(TableName.new(CompilerCatalogTable::COMPILERSCOPE,"myTableFunction"))
    return r
  end

  def test_notin
    utterance = "program path;

    "


  end
  def test_watch
    prep("program foo;
    define(arg,keys(0,1),{String,String,Integer});
    define(link,keys(0,1),{String,String,Integer});
    //watch(arg,id);
    arg(A,B,C) :- link(A,B), C := 1;
    ")

  end
  def test_materialization
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
    assert_equal(["here", "there"],result.tups[0].values)
  end

  def test_prog2
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
    utterance = "program agg_test;
    define(link,keys(0,1),{String,String,Integer,String});
    define(min_cost,keys(0,1),{String,String,Float});
    define(counter,keys(0,1),{String,String,Integer});
    min_cost(From,To,avg<Cost>) :- link(From,To,Cost,Annotation);
    counter(From,To,count<Cost>) :- link(From,To,Cost,Annotation);
    "
    prog = prep(utterance)
    tn, ts = gen_link_tuples("agg_test")
    q0 = prog.get_queries(tn)[0]
    q1 = prog.get_queries(tn)[1] 
    q0, q1 = q1, q0 if q1.output.name.name == "min_cost"
      
    result = q0.evaluate(ts)
    assert_equal(result.tups.length, 2)
    result.tups.each do |t|
      assert(t.values == ["N1","N2",10] || t.values == ["N2","N3",7.5])
    end
    #assert_equal(result.tups[0].values, ["N1","N2",10.0])
    #assert_equal(result.tups[1].values, ["N2","N3",7.5])
    result = q1.evaluate(ts)
    assert_equal(result.tups.length, 2)
    result.tups.each do |t|
      assert(t.values == ["N1","N2",1] || t.values == ["N2","N3",2])
    end
  end




# fact processing won't work just via planning ... 
  # def test_event_agg
  #   utterance = "program agg_test;
  #   define(foo, {String});
  #   define(link,keys(0,1),{String,String,Integer,String});
  #   link(\"a\",\"b\",2,\"hi\");
  #   tyson min_cost(From,To,avg<Cost>) :- foo(X), link(From,To,Cost,Annotation);"
  #   prog = prep(utterance)
  #   p = prog.plan
  # 
  #   # require 'ruby-debug'; debugger
  #   link = @runtime.catalog.table("agg_test::link")
  #   assert_equal(["a","a",2,"hi"], link.tuples.tups[0].values)
  # 
  #   foo_tup = Tuple.new("go")
  #   v1 = Variable.new("Col", String, 0,nil)
  #   foo_tup.schema = Schema.new("foo_schema", [v1])
  #   tn = TableName.new("agg_test", "foo")
  #   ts = TupleSet.new(tn, foo_tup)
  #   result = prog.get_queries(tn)[0].evaluate(ts)
  #   assert_equal(1, result.tups.length)
  #   assert_equal(["a","a",2,"hi"], result.tups[0].values)
  # end

  def test_event
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

  def test_delete
    utterance = "program deltest;
    define(link,keys(0,1),{String, String, Integer, Tuple});
    define(path,keys(0,1),{String, String, Integer, Tuple});
    path(A,B,C,T) :- link(A, B, C, T);
    delete link(A,B,C,T) :- link(A,B,C,T), C > 5;
    "
    prog = prep(utterance)
    tn, ts = gen_link_tuples("deltest")

    q0 = prog.get_queries(tn)[0]
    q1 = prog.get_queries(tn)[1] 
    q0, q1 = q1, q0 if q1.output.name.name == "path"

    result = q0.evaluate(ts)
    assert_equal(2, result.tups.length)
    result.tups.each do |t|
      assert( (t.values == ["N1","N2",10,"first"]) || (t.values == ["N2","N3",5,"second"])) 
    end
    result = q1.evaluate(ts)
    assert_equal(1, result.tups.length)
    assert_equal(result.tups[0].values, ["N1","N2",10,"first"])
  end

### Fact processing won't work just via planning
  # def test_facts
  #   utterance = "program path;
  #   define(link,keys(0,1),{String,String,Integer,String});
  #   link(\"A\",\"B\",3,\"link 1\");
  #   link(\"B\",\"C\",2,\"link 2\");
  #   path(From,To,Cost) :- link(From,To,Cost,Annotation);
  #   "
  # 
  #   prog = prep(utterance)
  #   p = prog.plan
  # 
  #   link = @runtime.catalog.table("path::link")
  # 
  #   assert_equal(2, Compiler.fact.tuples.tups.size)
  #   assert_equal(2, link.tuples.tups.size)
  #   link.tuples.tups.each do |t|
  #     assert(t.to_s == "<A, B, 3, link 1>" || t.to_s == "<B, C, 2, link 2>")
  #   end   
  # 
  # end

  def test_assign
    utterance = "program path;
    define(link,keys(0,1),{String,String,Integer,String});
    echo(X) :- link(F,T,C,A), B := Array.new(), J := 2, X:= B.push(J);"
    prog = prep(utterance)

    ### require 'ruby-debug'; debugger
    tn, ts = gen_link_tuples("path")

    assert(!prog.get_queries(tn).nil?)
    result = prog.get_queries(tn)[0].evaluate(ts)
    # only 1 distinct tuple is generated
    assert_equal(result.tups.length, 1)
    assert_equal(result.tups[0].values, [[2]])

  end
end
