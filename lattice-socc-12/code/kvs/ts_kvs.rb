require 'rubygems'
gem 'minitest'  # Use the rubygems version of MT, not builtin (if on 1.9)
require 'minitest/autorun'

require './kvs'
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

module LatticeTestSugar
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

  def pair(x, y)
    PairLattice.new([x, y])
  end

  def unwrap_map(m)
    m.merge(m) {|k,v| v.reveal}
  end
end

class TestPair < MiniTest::Unit::TestCase
  include LatticeTestSugar

  def unwrap_pair(i, sym)
    val = i.send(sym).current_value.reveal
    [val.first.reveal, val.last.reveal]
  end

  def test_pair_max
    i = SimplePair.new
    i.p2 <+ pair(max(5), max(0))
    i.p3 <+ pair(max(4), max(10))
    i.tick
    assert_equal([5, 0], unwrap_pair(i, :p1))
  end

  def test_pair_set
    i = SimplePair.new
    i.p2 <+ pair(set(1, 2, 3), set(4, 5, 6))
    i.p3 <+ pair(set(1, 2), set(7, 8, 9))
    i.tick
    first, last = unwrap_pair(i, :p1)
    assert_equal([1, 2, 3], first.sort)
    assert_equal([4, 5, 6], last.sort)

    i.p2 <+ pair(set(4), set(4, 25))
    i.tick
    first, last = unwrap_pair(i, :p1)
    assert_equal([1, 2, 3, 4], first.sort)
    assert_equal([4, 5, 6, 25], last.sort)

    i.p3 <+ pair(set(1, 2, 3, 4, 5), set(10))
    i.tick
    first, last = unwrap_pair(i, :p1)
    assert_equal([1, 2, 3, 4, 5], first.sort)
    assert_equal([10], last.sort)

    i.p3 <+ pair(set(1, 2, 3, 4, 5, 6), set())
    i.tick
    first, last = unwrap_pair(i, :p1)
    assert_equal([1, 2, 3, 4, 5, 6], first.sort)
    assert_equal([], last.sort)
  end

  def test_pair_vc
    i = SimplePair.new
    i.p2 <+ pair(map("k" => max(1)), set(20))
    i.p3 <+ pair(map(), set(1, 2, 3))
    i.tick
    first, last = unwrap_pair(i, :p1)
    first_plain = unwrap_map(first)
    assert_equal({"k" => 1}, first_plain)
    assert_equal([20], last.sort)

    i.p2 <+ pair(map("l" => max(2)), set(21, 22))
    i.p3 <+ pair(map("j" => max(3)), set(23))
    i.tick
    first, last = unwrap_pair(i, :p1)
    first_plain = unwrap_map(first)
    assert_equal([["j", 3], ["k", 1], ["l", 2]], first_plain.sort)
    assert_equal([20, 21, 22, 23], last.sort)

    i.p2 <+ pair(map("k" => max(1), "l" => max(2), "j" => max(4)), set(9, 99))
    i.tick
    first, last = unwrap_pair(i, :p1)
    first_plain = unwrap_map(first)
    assert_equal([["j", 4], ["k", 1], ["l", 2]], first_plain.sort)
    assert_equal([9, 99], last.sort)
  end
end

class TestMergeMapKvs < MiniTest::Unit::TestCase
  include LatticeTestSugar

  def test_simple
    r = MergeMapKvsReplica.new
    r.run_bg
    c = KvsClient.new(r.ip_port)
    c.run_bg

    c.write('foo', max(5))
    res = c.read('foo')
    assert_equal(5, res.reveal)

    c.write('foo', max(3))
    res = c.read('foo')
    assert_equal(5, res.reveal)

    c.write('foo', max(7))
    res = c.read('foo')
    assert_equal(7, res.reveal)

    c.stop_bg
    r.stop_bg
  end
end

class TestVectorClockKvs < MiniTest::Unit::TestCase
  include LatticeTestSugar

  def test_simple
    r = VectorClockKvsReplica.new
    r.run_bg
    c = KvsClient.new(r.ip_port)
    c.run_bg

    c.write('foo', pair(map, set(1)))
    res = c.read('foo')
    # XXX: seems wrong
    assert_equal({r.ip_port => 0}, unwrap_map(res.fst.reveal))
    assert_equal([1], res.snd.reveal)

    c.stop_bg
    r.stop_bg
  end
end
