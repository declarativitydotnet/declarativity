require "lib/types/table/key"
require "lib/types/basic/tuple"
require "test/unit"
require "rubygems"

class TestKey < Test::Unit::TestCase
  def default_test
    k = Key.new(0,1)
    l = Key.new()
    m = Key.new(1,2)
    assert_equal(k == k, true)
    assert_equal(k == l, false)
    assert_equal(k.to_s, "0, 1")
    assert_equal(l.to_s, "None")
    k.each_with_index {|a, i| assert_equal(a.to_s, i.to_s)}
    assert(l < k)
    assert(k > l)
    assert(k < m)
    assert(m >= m)
    k << (3)
    assert_equal(k.size,3)
    
    t = Tuple.new(1, 2, 3, 4)
    assert_equal(l.project(t), t)
    assert_equal(k.project(t).values, [1, 2, 4])
  end
end