require "lib/types/basic/tuple_set"
require "lib/types/basic/tuple"
require "test/unit"
require "rubygems"

class TestTupleSet < Test::Unit::TestCase
  def default_test
    a = Array.new
    a << Tuple.new(1,"test")
    a << Tuple.new(2, "test2")
    a << Tuple.new(3, "test3")
    tn = TableName.new("global","JoeSet")
    ts = TupleSet.new(tn, *a)


    assert_equal(ts.size, ts.tups.length)
    assert_equal(3,ts.tups.length)
    assert(ts.ts_id =~ /TupleSet:[0-9]*/)

    #assert(ts.to_s =~ /TupleSet:[0-9]*#<TupleSet:.*/)
    assert_equal(ts,ts)
    assert_not_equal(ts,a)
    assert_equal(ts<=>ts,0)

    assert_equal(ts.tups[0].values, [1,"test"])    
    assert_equal(ts.tups[1].values, [2,"test2"])    
    assert_equal(ts.tups[2].values, [3,"test3"])    
    assert_equal(ts.delete(a[2]), a[2])
    b = Array.new
    assert_equal(ts.tups[0].values, [1,"test"])
    assert_equal(ts.tups[1].values, [2,"test2"])    
    assert_equal(ts.size, 2)
    ts << a[2]
    assert_equal(ts.size, 3)    
    
    ts.each { |t| assert_equal(t.count, 1) }


	

  end
end
    
