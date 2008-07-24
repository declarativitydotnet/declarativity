require "lib/core/system"
require "lib/types/table/table"
require 'test/unit'
require "rubygems"

class TestSystem < Test::Unit::TestCase
  def default_test
    sys = System.new
    sys.init
    
    # dorky coverage
    assert_equal(System.clock.current, -1)
    assert_equal(System.query.table_type, 1)
    assert_equal(System.periodic.table_type, 1)
    assert_equal(System.program("chord"), nil)
    assert_equal(System.program_np("chord", nil), nil)

    # sys.bootstrap
    # sys.main
  end
end