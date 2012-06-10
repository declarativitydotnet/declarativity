require 'rubygems'
gem 'minitest'  # Use the rubygems version of MT, not builtin (if on 1.9)
require 'minitest/autorun'

require './lpair'

class SimplePair
  include Bud

  state do
    lpair :p1
    lpair :p2
    lpair :p3
  end

  bloom do
    p1 <= p2
    p1 <= p3
  end
end

class TestPair < MiniTest::Unit::TestCase
  def max(x)
    Bud::MaxLattice.new(x)
  end

  def set(*x)
    Bud::SetLattice.new(x)
  end

  def map(x={})
    raise unless x.kind_of? Hash
    Bud::MapLattice.new(x)
  end

  def unwrap_pair(i, sym)
    val = i.send(sym).current_value.reveal
    [val.first.reveal, val.last.reveal]
  end

  def unwrap_map(m)
    m.merge(m) {|k,v| v.reveal}
  end

  def test_pair_max
    i = SimplePair.new
    i.p2 <+ PairLattice.new([max(5), max(0)])
    i.p3 <+ PairLattice.new([max(4), max(10)])
    i.tick
    assert_equal([5, 0], unwrap_pair(i, :p1))
  end

  def test_pair_set
    i = SimplePair.new
    i.p2 <+ PairLattice.new([set(1, 2, 3), set(4, 5, 6)])
    i.p3 <+ PairLattice.new([set(1, 2), set(7, 8, 9)])
    i.tick
    first, last = unwrap_pair(i, :p1)
    assert_equal([1, 2, 3], first.sort)
    assert_equal([4, 5, 6], last.sort)

    i.p2 <+ PairLattice.new([set(4), set(4, 25)])
    i.tick
    first, last = unwrap_pair(i, :p1)
    assert_equal([1, 2, 3, 4], first.sort)
    assert_equal([4, 5, 6, 25], last.sort)

    i.p3 <+ PairLattice.new([set(1, 2, 3, 4, 5), set(10)])
    i.tick
    first, last = unwrap_pair(i, :p1)
    assert_equal([1, 2, 3, 4, 5], first.sort)
    assert_equal([10], last.sort)

    i.p3 <+ PairLattice.new([set(1, 2, 3, 4, 5, 6), set()])
    i.tick
    first, last = unwrap_pair(i, :p1)
    assert_equal([1, 2, 3, 4, 5, 6], first.sort)
    assert_equal([], last.sort)
  end

  def test_pair_vc
    i = SimplePair.new
    i.p2 <+ PairLattice.new([map(:k => max(1)), set(1)])
    i.p3 <+ PairLattice.new([map(), set(1, 2, 3)])
    i.tick
    i.tick
    first, last = unwrap_pair(i, :p1)
    first_plain = unwrap_map(first)
    assert_equal({:k => 1}, first_plain)
    assert_equal([1], last.sort)
  end
end
