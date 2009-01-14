require "lib/core/runtime"
require "lib/types/table/table"
require 'test/unit'
require 'lib/lang/compiler'  # require to initialize catalog tables
require "rubygems"

class TestSystem < Test::Unit::TestCase
  def default_test
    Runtime.main(10001, 'test/unit/olg/path.olg')
  end
end
