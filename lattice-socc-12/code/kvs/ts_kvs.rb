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

  def unwrap_pair(i, sym)
    val = i.send(sym).current_value.reveal
    [val.first.reveal, val.last.reveal]
  end

  def test_pair_max
    i = SimplePair.new
    i.p2 <+ PairLattice.new([max(5), max(0)])
    i.p3 <+ PairLattice.new([max(4), max(10)])
    i.tick
    assert_equal([5, 0], unwrap_pair(i, :p1))
  end
end
