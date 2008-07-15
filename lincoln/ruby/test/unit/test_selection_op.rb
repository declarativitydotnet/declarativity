require "lib/types/table/table"
require "lib/types/table/index_table"
require "lib/types/operator/selection_op"
require "lib/lang/plan/boolean"
require "lib/lang/plan/selection_term"
require 'lib/types/function/filter'
require 'lib/lang/plan/arguments'
require 'lib/lang/plan/value'
require "test/unit"

class TestSelectionOp < Test::Unit::TestCase
  $catalog = Catalog.new
  $index = IndexTable.new

  def default_test
    constant_expr_test
    variable_expr_test
  end
  
  def constant_expr_test
    # test a few constant expressions
    v = Variable.new("id", Integer)
    v.position = 0
    v2 = Variable.new("name", String)
    v2.position = 1
    schemey = Schema.new("schemey", [v,v2])

    booleq = Boolean.new("==", Value.new(0), Value.new(0))
    seltermeq = SelectionTerm.new(booleq)
    selopeq = SelectionOp.new(seltermeq, schemey)
    assert_equal(selopeq.to_s, "SELECTION [(0 == 0)]")
    ts = TupleSet.new("test",Tuple.new(1,"hi"))
    assert_equal(selopeq.evaluate(ts).tups, ts.tups)
    
    boolneq = Boolean.new("==", Value.new(0), Value.new(1))
    seltermneq = SelectionTerm.new(boolneq)
    selopneq = SelectionOp.new(seltermneq, [Integer,String])
    ts = TupleSet.new("test",Tuple.new(1,"hi"))
    assert_equal(selopneq.evaluate(ts).tups, [])

    boollt = Boolean.new("<", Value.new(0), Value.new(1))
    seltermlt = SelectionTerm.new(boollt)
    seloplt = SelectionOp.new(seltermlt, [Integer,String])
    ts = TupleSet.new("test",Tuple.new(1,"hi"))
    assert_equal(seloplt.evaluate(ts).tups, ts.tups)
    
    # miscellaneous tests for coverage
    assert_equal(seloplt.requires, [])
    assert_equal(boollt.type, boollt.function.returnType)
  end
  def variable_expr_test
    t = Tuple.new(1, "joe")
    v = Variable.new("id", Integer)
    v.position = 0
    v2 = Variable.new("name", Numeric)
    v2.type = String
    assert_equal(v2.function.returnType, String)
    v2.position = 1
    schemey = Schema.new("schemey", [v,v2])
    t.schema = schemey
    
    assert_equal(v.function.evaluate(t), 1)
    assert_equal(v2.function.evaluate(t), "joe")
    
    one = Value.new(1)
    assert((one.type < v.type) || (v.type < one.type))
    assert_equal(one.function.returnType, one.type)
    
    booleq = Boolean.new("==", v, Value.new(1))
    seltermeq = SelectionTerm.new(booleq)
    selopeq = SelectionOp.new(seltermeq, schemey)
    
    assert_equal(selopeq.schema, t.schema)
    
    ts = TupleSet.new("test",t)
    assert_equal(selopeq.evaluate(ts).tups, ts.tups)
    
    assert_equal(selopeq.requires, [v])
  end
end