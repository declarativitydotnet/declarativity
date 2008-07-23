require 'lib/lang/plan/rule'
require 'lib/lang/plan/predicate'
require 'lib/lang/plan/value'
require 'lib/lang/plan/selection_term'
require 'lib/core/system'
require "test/unit"
require "rubygems"

class TestRule < Test::Unit::TestCase
  def default_test
    sys = System.new
    sys.init

    # set up a table link(from, to, cost, annotation)
    v1 = Variable.new("from", Integer)
    v1.position = 0
    v2 = Variable.new("to", Integer)
    v2.position = 1
    v3 = Variable.new("cost", Float)
    v3.position = 2
    v4 = Variable.new("annotation", String)
    v4.position = 3
    link = Predicate.new(false, TableName.new(nil, "link"), nil, [v1, v2, v3, v4])
    link.set('testprog', 'r1', 1)

    t1 = Tuple.new(1,2,0.5, "first")
    t2 = Tuple.new(2,3,1.0, "second")
    schema1 = Schema.new("schema1", [v1,v2,v3,v4])
    t1.schema = schema1
    t2.schema = schema1
    tn = TableName.new(nil, "link")
    ts = TupleSet.new(tn, t1, t2)
    require 'ruby-debug'; debugger
    table = BasicTable.new(tn, 4, Table::INFINITY, Key.new(1), [Integer,Integer,Float,String])
      
    # simple projection rule
    head = Predicate.new(false, TableName.new(nil,"head"), nil, [v1, v2, v3])
    head.set('testprog', 'r1', 0)
    body = [link]
    r = Rule.new(1, 'r1', true, false,  head, body)
    assert_equal(r.to_s, "public r1 ::head(from:0, to:1, cost:2) :- \n\t::link(from:0, to:1, cost:2, annotation:3);\n\t;\n") 
    result = r.query(nil)[0].evaluate(ts)
    assert_equal(result.tups[0].values, [1,2,0.5])
    assert_equal(result.tups[1].values, [2,3,1.0])
    
    # selection rule
    bool = Boolean.new("==", v2, Value.new(1))
    selterm = SelectionTerm.new(bool)
    body = [link, selterm]
    
    r = Rule.new(1, 'r1', true, false,  head, body)
    require 'ruby-debug'; debugger
    result = r.query(nil)[0].evaluate(ts)
    assert_equal(result.size, 1)
    assert_equal(result.tups[0].values, [1,2,0.5])
  end
end