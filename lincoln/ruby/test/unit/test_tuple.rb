require "lib/types/basic/tuple"
require "test/unit"
require "rubygems"

class TestTuple < Test::Unit::TestCase
  def setup
    @t = Tuple.new(1, "Joe")
    a = [Variable.new("eid", Integer, 0,nil)]
    
    # test schema not matching tuple arity
    assert_raise(RuntimeError) {
      s = Schema.new("emp", a)
      @t.schema = s
    }
    
    # now get schema array to match tuple arity
    a << Variable.new("name", String, 1,nil)
    assert_nothing_raised(RuntimeError) {
      s2 = Schema.new("emp", a)
      @t.schema = s2
    }
    
    v = Variable.new("ssn", Integer, 2,nil)
    @t.append(v, 123456789)
    assert_equal(@t.to_s, "<1, Joe, 123456789>")
  end
  
  def test_simple
    r = Tuple.new(1, "Joe")
    r.tid = 17
    assert_equal(r.tid, 17) 
    assert_equal(@t.values[0), @t.name_value("eid")]   
    assert_equal(@t.values[1), @t.name_value("name")]   
    assert_equal(@t.values[2), @t.name_value("ssn")]  
    assert_equal(@t.tuple_type("eid"), Integer) 
    assert_equal(@t.count, 1)
    @t.count = 0
    assert_equal(@t.count, 0)
    @t.count = 1
    @t.timestamp = Time.now
    assert_not_equal(@t.timestamp, Time.now)
  end
  
  def test_compare_and_join
    assert_equal(@t <=> @t, 0)
    # 0-length tuples
    s = Tuple.new
    t = Tuple.new
    assert_equal(s, t)
    assert_equal(s.join(t), s)
    assert_equal(t.join(s), s)
    assert_equal(s.join(t).size, 0)

    # different sized tuples
    c1 = Variable.new("eid", Integer, 0,nil)
    t.append(c1, 1)
    assert_equal(@t <=> t, -1)
    
    # same sizes, differing values
    c2 = Variable.new("name", String, 1,nil)
    t.append(c2, "Joe")
    c3 = Variable.new("ssn", Integer, 2,nil)
    t.append(c3, 123456790)
    t.schema = Schema.new("Gosh", [c1, c2, c3])
    assert_equal(@t <=> t, -1)

    # one value is nil
    t.set_value(c3, nil)
    assert_equal(@t <=> t, -1)
    
    # add a value with no position
    c4 = Variable.new("hobby", String,nil)
    t.set_value(c4, "bowling")
    assert_equal(t.values[3], "bowling")
    t.schema.variable("hobby").position = 3
    
    # test join
    assert_nil(@t.join(t)) # join fails due to nil in t.ssn
    t.set_value(2, 123456790)
    assert_nil(@t.join(t)) # join fails due to mismatch on values in ssn
    t.set_value(2, 123456789)
    assert_equal(@t.join(t).size, 4) # join succeeds
    assert_equal(t.join(@t).size, 4) # join succeeds
    t.set_value(1, nil)
    @t.set_value(1, nil)
    assert_equal(@t.join(t).size, 4) # join succeeds
    @t.set_value("name", "Joe")
  end

  def test_join_symmetry
    t1 = Tuple.new
    t2 = Tuple.new
    c1 = Variable.new("x", Integer, 1,nil)
    t1.append(c1, nil)

    assert_equal(t2.join(t1), t1)
    assert_equal(t1.join(t2), t1)
    assert_not_equal(t2.join(t1), t2)
    assert_not_equal(t1.join(t2), t2)

    t2.append(c1.clone, nil)
    c2 = Variable.new("y", String, 2,nil)
    t1.append(c2, "xyz")

    assert_equal(t1.join(t2), t1)
    assert_equal(t2.join(t1), t1)
    assert_not_equal(t1.join(t2), t2)
    assert_not_equal(t2.join(t1), t2)

    t2.append(c2, nil)
    assert_nil(t1.join(t2))
    assert_nil(t2.join(t1))
  end
end
