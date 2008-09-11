require "lib/types/basic/tuple"
require "test/unit"
require "rubygems"

class TestTuple < Test::Unit::TestCase
  def setup
    @t = Tuple.new(1, "Joe")
    a = [Variable.new("eid", Integer)]
    a[0].position=0
    
    # test schema not matching tuple arity
    assert_raise(RuntimeError) {
      s = Schema.new("emp", a)
      @t.schema = s
    }
    
    #now get schema array to match tuple arity
    a << Variable.new("name", String)
    a[1].position=1
    assert_nothing_raised(RuntimeError) {
      s2 = Schema.new("emp", a)
      @t.schema = s2
    }
    
    v = Variable.new("ssn", Integer)
    v.position=2
    @t.append(v, 123456789)
    assert_equal(@t.to_s, "<1, Joe, 123456789>")
  end
  
  def test_simple
    r = Tuple.new(1, "Joe")
    r.tid = 17
    assert_equal(r.tid, 17) 
    assert_equal(@t.value(0), @t.value("eid"))   
    assert_equal(@t.value(1), @t.value("name"))   
    assert_equal(@t.value(2), @t.value("ssn"))  
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
    s = t = Tuple.new
    assert_equal(s <=> t, 0)
    assert(s==t)
    
    # different sized tuples
    c1 = Variable.new("eid", Integer)
    c1.position = 0
    t.append(c1, 1)
    assert_equal(@t <=> t, -1)
    
    # same sizes, differing values
    c2 = Variable.new("name", String)
    c2.position = 1
    t.append(c2, "Joe")
    c3 = Variable.new("ssn", Integer)
    c3.position = 2
    t.append(c3, 123456790)
    t.schema = Schema.new("Gosh", [c1, c2, c3])
    assert_equal(@t <=> t, -1)

    # one value is nil
    t.set_value(c3, nil)
    assert_equal(@t <=> t, -1)
    
    # add a value with no position
    c4 = Variable.new("hobby", String)
    t.set_value(c4, "bowling")
    assert_equal(t.values[3], "bowling")
    t.schema.variable("hobby").position = 3
    
    # test join
    assert_equal(@t.join(t), nil) # join fails due to nil in t.ssn
    t.set_value(2, 123456790)
    assert_equal(@t.join(t), nil) # join fails due to mismatch on values in ssn
    t.set_value(2, 123456789)
    assert_equal(@t.join(t).size, 4) # join succeeds 
    assert_equal(t.join(@t).size, 4) # join succeeds 
    t.set_value(1, nil)
    @t.set_value(1, nil)
    assert_equal(@t.join(t).size, 4) # join succeeds 
    @t.set_value("name", "Joe")
  end
  
  end
