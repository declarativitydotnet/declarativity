require "lib/types/table/table"
require "lib/types/table/index_table"
require "lib/types/operator/scan_join"
require "lib/lang/plan/boolean"
require "lib/lang/plan/selection_term"
require 'lib/types/function/filter'
require 'lib/lang/plan/arguments'
require 'lib/lang/plan/value'
require 'lib/lang/plan/predicate'
require "test/unit"

class TestJoin < Test::Unit::TestCase
  $catalog = Catalog.new
  $index = IndexTable.new

  def default_test
    t1 = Tuple.new(1, "joe")
    v = Variable.new("id", Integer)
    v.position = 0
    v2 = Variable.new("name", String)
    v2.position = 1
    schema1 = Schema.new("schema1", [v,v2])
    t1.schema = schema1
    
    t2 = Tuple.new(1, "hellerstein")
    v3 = Variable.new("id", Integer)
    v3.position = 0
    v4 = Variable.new("lastname", String)
    v4.position = 1
    schema2 = Schema.new("schema2", [v3,v4])
    t2.schema = schema2

    table1 = BasicTable.new('Firstname', 10, BasicTable::INFINITY, Key.new(0), [Integer, String])
    ts = TupleSet.new('fnames', t1)
    table1.insert(ts, nil)
    table2 = EventTable.new('Lastname', [Integer, String])
    ts2 = TupleSet.new('lnames', t2)


    test_join(table2, schema2, t1, t2, v)
    test_scan_join(table1, schema1, ts, schema2, ts2)
  end

  # all we're doing here is testing that the Join operators checks 
  # single-table stuff correctly.  No joining actually happens.
  def test_join(table, schema, t1, t2, v)
    # test constant matches
    constant = Value.new("hellerstein")
    constant.position = 1
    pred = Predicate.new(false, table.name, table, [constant])
    pred.set("myprog", "r1", 1)
    join = Join.new(pred, schema)
    assert(join.validate(t1,t2))

    # test constant that does not match
    constant2 = Value.new("jones")
    constant2.position = 1
    pred2 = Predicate.new(false, table.name, table, [constant2])
    pred2.set("myprog", "r2", 1)
    join2 = Join.new(pred2, schema)
    assert_equal(join2.validate(t1,t2), false)
    
    # test repeated variable
    constant2 = Value.new("jones")
    constant2.position = 1
    pred2 = Predicate.new(false, table.name, table, [v,v])
    pred2.set("myprog", "r2", 1)
    join2 = Join.new(pred2, schema)
    assert(join2.validate(t1,t2))
    
    #coverage
    assert_equal(join2.filters(pred2)[0].lhs.returnType, join2.filters(pred2)[0].rhs.returnType)
    assert_equal(join2.schema.variables, schema.variables)
    assert_equal(join2.requires, {})    
  end 
  
  def test_scan_join(basic_table, basic_schema, ts1, event_schema, ts2)
    pred = Predicate.new(false,basic_table.name,basic_table, basic_schema.variables)
    pred.set("myprog", "r3", 1) 
    sj = ScanJoin.new(pred, basic_schema)
    assert_equal(sj.evaluate(ts2).tups.length, 1)
    assert_equal(sj.evaluate(ts2).tups[0].values, [1, "hellerstein", "joe"])    
    ts2 << ts2.tups[0]
    assert_equal(sj.evaluate(ts2).tups.length, 2)
    basic_table.insert(ts1, nil)
    assert_equal(sj.evaluate(ts2).tups.length, 4)
  end
end
