require "lib/types/table/event_table"
require "lib/types/basic/tuple_set"
require "lib/types/basic/tuple"
require "test/unit"
require "rubygems"

class TestEventTable < Test::Unit::TestCase
  def default_test
    $catalog=nil; $index=nil
    et = EventTable.new('Orli', [Integer, String])
    ts = TupleSet.new('test', Tuple.new(1, 'hi'), Tuple.new(2, 'bye'))
    assert_equal(et.delete(ts).size, 2)
    assert_raise(RuntimeError) {et.delete(Tuple.new(1,'hi'))}
    assert_equal(et.insert(ts, nil).size, 2)
    assert(et.primary.nil?)
    assert(et.secondary.nil?)
    assert(et.tuples.nil?)
    assert_equal(et.cardinality, 0)
    assert_equal(et.to_s, "Orli, IntegerString, 0, 0, keys(), {IntegerString}\n")
    assert_equal(et.table_type, Table::TableType::EVENT)
    
    # todo: check semantics of Event table are correct (no persistence)
  end
end