require "lib/types/table/table"
require "lib/types/table/event_table"
require "lib/types/operator/scan_join"
require "lib/types/operator/index_join"
require "lib/lang/plan/boolean"
require "lib/lang/plan/selection_term"
require 'lib/types/function/filter'
require 'lib/lang/plan/arguments'
require 'lib/lang/plan/value'
require 'lib/lang/plan/predicate'
require 'lib/lang/parse/schema'
require 'lib/core/system'
require "test/unit"

class TestJoin < Test::Unit::TestCase
  $catalog=nil; $index=nil
  sys = System.new
  sys.init

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
    
    pred = Predicate.new(false,table1.name, Table::Event::NONE, schema1.variables)
    pred.set("myprog", "r3", 1) 
    
    sj = ScanJoin.new(pred, schema1)
    test_real_join(sj, table1, schema1, ts, schema2, ts2)
    assert_equal(sj.to_s, "NEST LOOP JOIN: PREDICATE[Firstname(id:0, name:1)]")

    $catalog=nil; $index=nil
    sys = System.new
    sys.init
    
    # reset the tables and tuplesets and repeat the experiment with index join
    table1 = BasicTable.new('Firstname', 10, BasicTable::INFINITY, Key.new(0), [Integer, String])
    ts = TupleSet.new('fnames', t1)
    table1.insert(ts, nil)
    table2 = EventTable.new('Lastname', [Integer, String])
    ts2 = TupleSet.new('lnames', t2)
    
    ij = IndexJoin.new(pred, schema1, Key.new(0), table1.primary)
    test_real_join(ij, table1, schema1, ts, schema2, ts2)
    assert_equal(ij.to_s, "INDEX JOIN: PREDICATE[Firstname(id:0, name:1)]")
    
  end

  # all we're doing here is testing that the Join operators checks 
  # single-table stuff correctly.  No joining actually happens.
  def test_join(table, schema, t1, t2, v)
    # test constant matches
    constant = Value.new("hellerstein")
    constant.position = 1
    pred = Predicate.new(false, table.name, Table::Event::NONE, [constant])
    pred.set("myprog", "r1", 1)
    join = Join.new(pred, schema)
    assert(join.validate(t1,t2))

    # test constant that does not match
    constant2 = Value.new("jones")
    constant2.position = 1
    pred2 = Predicate.new(false, table.name, Table::Event::NONE, [constant2])
    pred2.set("myprog", "r2", 1)
    join2 = Join.new(pred2, schema)
    assert_equal(join2.validate(t1,t2), false)

    # test repeated variable
    constant2 = Value.new("jones")
    constant2.position = 1
    pred2 = Predicate.new(false, table.name, Table::Event::NONE, [v,v])
    pred2.set("myprog", "r2", 1)
    join2 = Join.new(pred2, schema)
    assert(join2.validate(t1,t2))

    #coverage
    assert_equal(join2.filters(pred2)[0].lhs.returnType, join2.filters(pred2)[0].rhs.returnType)
    assert_equal(join2.schema.variables, schema.variables)
    assert_equal(join2.requires, {})    
  end 

  def test_real_join(join, basic_table, basic_schema, ts1, event_schema, ts2)
    # 1 tuple on each side, should join to produce one tuple 
    assert_equal(join.evaluate(ts2).tups.length, 1)
    assert_equal(join.evaluate(ts2).tups[0].values, [1, "hellerstein", "joe"])    

    # add another matching tuple to ts2, should join to produce 2 tuples
    tmatch = Tuple.new(1, "huckenfluster")
    v3 = Variable.new("id", Integer)
    v3.position = 0
    v4 = Variable.new("lastname", String)
    v4.position = 1
    schema2 = Schema.new("schema2", [v3,v4])
    tmatch.schema = schema2
    ts2 << tmatch
    assert_equal(join.evaluate(ts2).tups.length, 2)

    # add another matching tuple to basic_table, should join to produce 4
    ttablematch = Tuple.new(1, "sally")
    v = Variable.new("id", Integer)
    v.position = 0
    v2 = Variable.new("name", String)
    v2.position = 1
    schema1 = Schema.new("schema1", [v,v2])
    ttablematch.schema = schema1
    
    
    basic_table.insert(TupleSet.new("stuff", ttablematch), nil)
    assert_equal(join.evaluate(ts2).tups.length, 4)

    # add a non-matching tuple to ts2, should not change output
    t3 = Tuple.new(2, "sacks")
    t3.schema = event_schema
    ts2 << t3
    assert_equal(join.evaluate(ts2).tups.length, 4)

    # add a  tuple to basic_table that natches t3, should create one more output
    t4 = Tuple.new(2, "adene")
    t4.schema = basic_schema
    ts4 = TupleSet.new("test", t4)
    basic_table.insert(ts4, nil)
    assert_equal(join.evaluate(ts2).tups.length, 5)
    join.evaluate(ts2).tups.each do |t|
      assert_equal(t.values, [2, "sacks", "adene"]) if t.values[0] == 2
    end
    
    # add a non-matching tuple to basic_table, should not change output
    t5 = Tuple.new(3, "smith")
    t5.schema = basic_schema
    ts5 = TupleSet.new("test", t5)
    basic_table.insert(ts5, nil)
    assert_equal(join.evaluate(ts2).tups.length, 5)
  end
  
end
