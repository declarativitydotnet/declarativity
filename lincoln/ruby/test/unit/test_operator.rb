require "lib/types/operator/operator"
require "test/unit"
require "rubygems"

class TestOperator < Test::Unit::TestCase
  def default_test
    # undefined superclass interfaces
    o = Operator.new(nil,nil)
    assert_raise(RuntimeError){o.to_s}
    assert_raise(RuntimeError){o.evaluate(nil)}
    assert_raise(RuntimeError){o.schema}
    assert_raise(RuntimeError){o.requires}
    assert_equal(o <=> o, 0)
  end
end