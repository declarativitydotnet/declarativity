require "lib/types/table/ref_table"
require "lib/types/basic/tuple"
require "lib/types/table/catalog"
require "test/unit"
require "rubygems"

class TestRefTable < Test::Unit::TestCase
  def default_test
    $catalog=nil; $index=nil
    sys = System.new
    sys.init
    
    rt = RefTable.new("Family", Key.new(0), [Integer, String])
    dd = Tuple.new(1, "Dahlia")
    o = Tuple.new(2, "Orli")
    ts = TupleSet.new("tups", dd, o, Tuple.new(3, "Dad"))
    assert_equal(rt.insert(ts, nil).size, 3)
    
    # now add a second copy of Dahlia
    dd_ts = TupleSet.new("tups", dd)
    assert_equal(rt.insert(dd_ts, nil).size, 0) # no new tups inserted
    
    # and delete a ref to Dahlia, so there's only one.  No tuple deletion.
    assert_equal(rt.delete([dd]).size, 0)
    
    assert_equal(rt.cardinality,3)
    
    # now delete the last ref to Dahlia, so there's zero.  Dahlia deleted.
    assert_equal(rt.delete([dd]).size, 1)
    assert_equal(rt.cardinality,2)
    
    
    assert_equal(rt.delete([dd]).size, 0)
    assert_equal(rt.cardinality,2)
    assert_equal(rt.secondary.length, 0)
    rt.drop
  end
end