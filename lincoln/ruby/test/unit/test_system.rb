require "lib/core/system"
require "lib/types/table/table"
require 'test/unit'
require 'lib/lang/compiler'  # require to initialize catalog tables
require "rubygems"

class TestSystem < Test::Unit::TestCase
  def default_test
    $catalog = nil
    $index = nil
    sys = System.new
    sys.init
    
    # dorky coverage
    assert_equal(System.clock.current, -1)
    assert_equal(System.query.table_type, 1)
    assert_equal(System.periodic.table_type, 1)
    assert_equal(System.program("chord"), nil)
    assert_equal(System.install_program("chord", nil), nil)

    sys.bootstrap # gets called from main
    # sys.main('test/unit/olg/path.olg')
  end
end
