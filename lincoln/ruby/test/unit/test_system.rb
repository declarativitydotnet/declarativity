require "lib/core/system"
require "lib/types/table/table"
require 'test/unit'
require "rubygems"

class TestSystem < Test::Unit::TestCase
  def default_test
    sys = System.new
    sys.init
    
    # dorky coverage
    assert_equal(sys.clock.current, -1)
    assert_equal(sys.query.type, 1)
    assert_equal(sys.periodic.type, 1)
    assert_equal(sys.program("chord"), nil)
    assert_equal(sys.program_np("chord", nil), nil)

    # sys.bootstrap
    # sys.main
  end
end