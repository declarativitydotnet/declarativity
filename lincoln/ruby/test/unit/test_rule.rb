require 'lib/lang/plan/rule'
require 'lib/lang/plan/predicate'
require 'lib/core/system'
require "test/unit"
require "rubygems"

class TestRule < Test::Unit::TestCase
  def default_test
    sys = System.new
    sys.init
    v1 = Variable.new("from", Integer)
    v1.position = 0
    v2 = Variable.new("to", Integer)
    v2.position = 1
    v3 = Variable.new("cost", Float)
    v3.position = 2
    head = Predicate.new(false, "head", nil, [v1, v2, v3])
    head.set('testprog', 'r1', 0)
    
    link = Predicate.new(false, "link", nil, [v1, v2, v3])
    link.set('testprog', 'r1', 1)
    
    body = [link]
    r = Rule.new(1, 'r1', true, false,  head, body)
    
    assert_equal(r.to_s, nil)
  end
end