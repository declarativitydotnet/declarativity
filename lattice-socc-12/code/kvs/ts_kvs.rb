require 'rubygems'
gem 'minitest'  # Use the rubygems version of MT, not builtin (if on 1.9)
require 'minitest/autorun'

require './lpair'

class SimplePair
  state do
    lpair :p1
  end
end

class TestPair < MiniTest::Unit::TestCase
  def test_simple_pair
    i = SimplePair.new
  end
end
