require "lib/types/basic/tuple_set"
require "lib/types/basic/tuple"
require "test/unit"
require "rubygems"

class TestTupleSet < Test::Unit::TestCase
  def default_test
    a = Array.new
    a << Tuple.new(1,"test")
    a << Tuple.new(2, "test2")
    tn = TableName.new("global","JoeSet")
    ts = TupleSet.new(tn, *a)


    assert_equal(ts.size, ts.tups.length)
    assert_equal(2,ts.tups.length)
    assert(ts.ts_id =~ /TupleSet:[0-9]*/)

    #assert(ts.to_s =~ /TupleSet:[0-9]*#<TupleSet:.*/)
    assert_equal(ts,ts)
    assert_not_equal(ts,a)
    assert_equal(ts<=>ts,0)
    
    assert_equal(ts.delete(a[0]), a[0])
    assert_equal(ts.size, 1)
    ts << a[0]
    assert_equal(ts.size, 2)    
    
    ts.each { |t| assert_equal(t.count, 1) }


	

  end
end
    
