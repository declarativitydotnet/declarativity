require "lib/types/table/table"
require "lib/types/operator/selection_op"
require "lib/lang/plan/boolean"
require "lib/lang/plan/selection_term"
require 'lib/types/function/filter'
require 'lib/lang/plan/arguments'
require 'lib/lang/plan/value'
require 'lib/core/runtime'
require "test/unit"

class TestSelectionOp < Test::Unit::TestCase
  def default_test
    constant_expr_test
    variable_expr_test
  end
  
  def constant_expr_test
    # test a few constant expressions
    v = Variable.new("id", Integer, 0,nil)
    v2 = Variable.new("name", String, 1,nil)
    schemey = Schema.new("schemey", [v,v2])
    runtime = Runtime.new


    booleq = Boolean.new("==", Value.new(0), Value.new(0))
    seltermeq = SelectionTerm.new(booleq)
    selopeq = SelectionOp.new(runtime, seltermeq, schemey)
    assert_equal(selopeq.to_s, "SELECTION [(0 == 0)]")
    ts = TupleSet.new("test",Tuple.new(1,"hi"))
    assert_equal(selopeq.evaluate(ts).tups, ts.tups)
    
    boolneq = Boolean.new("==", Value.new(0), Value.new(1))
    seltermneq = SelectionTerm.new(boolneq)
    selopneq = SelectionOp.new(runtime, seltermneq, [Integer,String])
    ts = TupleSet.new("test",Tuple.new(1,"hi"))
    assert_equal(selopneq.evaluate(ts).tups, [])

    boollt = Boolean.new("<", Value.new(0), Value.new(1))
    seltermlt = SelectionTerm.new(boollt)
    seloplt = SelectionOp.new(runtime, seltermlt, [Integer,String])
    ts = TupleSet.new("test",Tuple.new(1,"hi"))
    assert_equal(seloplt.evaluate(ts).tups, ts.tups)
    
    # miscellaneous tests for coverage
    assert_equal(seloplt.requires, [])
    assert_equal(boollt.expr_type, boollt.function.returnType)
  end
  def variable_expr_test
    r = Runtime.new
    t = Tuple.new(1, "joe")
    v = Variable.new("id", Integer, 0,nil)
    v2 = Variable.new("name", Numeric, 1, nil)
    v2.expr_type = String
    assert_equal(v2.function.returnType, String)
    v2.position = 1
    schemey = Schema.new("schemey", [v,v2])
    t.schema = schemey
    
    assert_equal(v.function.evaluate(t), 1)
    assert_equal(v2.function.evaluate(t), "joe")
    
    one = Value.new(1)
    assert((one.expr_type < v.expr_type) || (v.expr_type < one.expr_type))
    assert_equal(one.function.returnType, one.expr_type)
    
    booleq = Boolean.new("==", v, Value.new(1))
    seltermeq = SelectionTerm.new(booleq)
    selopeq = SelectionOp.new(r, seltermeq, schemey)
    
    assert_equal(selopeq.schema, t.schema)
    
    ts = TupleSet.new("test",t)
    assert_equal(selopeq.evaluate(ts).tups, ts.tups)
    
    assert_equal(selopeq.requires, [v])
  end
end
