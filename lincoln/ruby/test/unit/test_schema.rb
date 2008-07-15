require "lib/types/basic/schema"
require "test/unit"

class TestSchema < Test::Unit::TestCase
  def test_simple
    a = [Variable.new("i", Integer), Variable.new("s", String), ]
    a[0].position = 1
    a[1].position = 2
    s = Schema.new("s1", a)
    t = Schema.new(s.name,s.variables)
    v = Variable.new("f", Float)
    v.position = 3
    s << v
    
    assert(s.contains(v))
    assert_equal(s.position("i"), 1)
    assert_equal(s.size, a.length+1)
    assert_equal(s.size, s.variables.length)
    assert_equal(s.types, [Integer,String,Float])
    assert_equal(s.name, "s1")
    assert_equal(s.to_s, "(i:1,s:2,f:3)")
    assert_equal(s.variable("f"),v)
    assert_equal(s.type("f"), Float)
    assert_not_equal(s.size,t.size)
    assert_equal(t.join(s).variables, s.variables)
  end
end