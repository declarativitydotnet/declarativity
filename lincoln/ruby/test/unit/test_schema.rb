require "lib/types/basic/schema"
require "test/unit"

class TestSchema < Test::Unit::TestCase
  def test_simple
    a = [Variable.new("i", Integer, 0,nil), Variable.new("s", String, 1,nil)]
    s = Schema.new("s1", a)
    t = Schema.new(s.name,s.variables)
    v = Variable.new("f", Float, 3,nil)
    s << v
    
    
    assert(s.contains(v))
    assert_equal(s.position("s"), 1)
    assert_equal(s.size, a.length+1)
    assert_equal(s.size, s.variables.length)
    assert_equal(s.types, [Integer,String,Float])
    assert_equal(s.name, "s1")
    assert_equal(s.to_s, "s1(i:0,s:1,f:2)")
    assert_equal(s.variable("f"),v)
    assert_equal(s.schema_type("f"), Float)
    assert_not_equal(s.size,t.size)
    assert_equal(t.join(s).variables, s.variables)

    news = Schema.new("new", nil)
    news.set_schema(s)
    assert_equal(s.name, news.name)
  end
end
