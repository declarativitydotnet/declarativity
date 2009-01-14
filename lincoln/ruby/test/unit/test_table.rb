require "lib/types/table/table"
require "lib/types/basic/tuple"
require "lib/types/table/key"
require "lib/types/table/catalog"
require "lib/types/table/hash_index"
require "lib/types/table/callback"
require "lib/core/runtime.rb"
require "test/unit"
require "rubygems"

class TestTable < Test::Unit::TestCase
  def test_default
    r = Runtime.new
    t = Table.new(r, 'Orli', Key.new(0), [Integer, String])

    # undefined superclass methods
    assert_raise(RuntimeError){t.tuples}
    assert_raise(RuntimeError){t.cardinality}
    assert_raise(RuntimeError){t.size}
    assert_raise(RuntimeError){t.primary}
    assert_raise(RuntimeError){t.secondary}

    assert_equal(t.types.to_s, "IntegerString")

    # silly coverage 
    t.unregister(Callback.new(t))

    assert_raise(RuntimeError){t.insert_tup(Tuple.new(1, 'hi'))}
    assert_raise(RuntimeError){t.delete_tup(Tuple.new(1, 'hi'))}
  end

  def test_delete
    r = Runtime.new
    tab = BasicTable.new(r, "foo",Key.new(0),[Integer,String])
    tup = Tuple.new(1,"one")
    tup1 = Tuple.new(2,"two")
    tup2 = Tuple.new(3,"three")
    ts = TupleSet.new("bar",tup,tup1,tup2)
    indx = HashIndex.new(r, tab,Key.new(1),String)
    tab.insert(ts,nil)
    ts2 = TupleSet.new("purge",tup2)

    assert_equal(3,tab.tuples.size)
    tab.delete(ts2)
    assert_equal(2,tab.tuples.size)
    tab.clear
    assert_equal(0,tab.tuples.size)
    res = indx.lookup(Tuple.new(nil,"two"))
    assert_equal(0,res.size)
  end
end
