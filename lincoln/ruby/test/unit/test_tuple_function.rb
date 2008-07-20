require "lib/types/function/tuple_function"
require 'test/unit'
require "rubygems"

class TestTupleFunction < Test::Unit::TestCase
  def default_test
    # dorky coverage
    tf = TupleFunction.new
    
    assert_raise(RuntimeError) {tf.evaluate(nil)}
    assert_raise(RuntimeError) {tf.returnType}
  end
end