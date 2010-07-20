class BabyBloom < Bloom
  def state
    scratch :scrtch, ['k1', 'k2'], ['v1', 'v2']
    scratch :scrtch2, ['k1', 'k2']
    table :tbl, ['k1', 'k2'], ['v1', 'v2']
  end
  
  def declaration
    strata[0] = rules {
      if bloomtime == 1 then
        scrtch <= [['a', 'b', 1, 2]]
        scrtch <= [['a', 'c', 3, 4]]
        scrtch2 <= [['a', 'b']]
        tbl <= [['a', 'b', 1, 2]]
        tbl <= [['z', 'y', 9, 8]]

        scrtch <+ [['c', 'd', 5, 6]]
        tbl <+ [['c', 'd', 5, 6]]
        tbl <- [['a', 'b', 1, 2]]
      end
    } 
  end
end

class DupKeyBloom < Bloom
  def state
    scratch :s, ['k'], ['name']
  end
  
  def declaration
    strata[0] = rules {
      s <= [[2000, 'bush']]
      s <= [[2000, 'gore']]
    }
  end
end

class DupTableBloom < Bloom
  def state
    scratch :s, ['k']
    scratch :s, ['l']
  end
  def declaration
  end
end

class DupColBloom < Bloom
  def state
    scratch :s ['a', 'a']
  end
  def declaration
  end
end

class TestCollections < Test::Unit::TestCase
 
  def test_simple_deduction
    program = BabyBloom.new('localhost', 12345)
    assert_nothing_raised( RuntimeError) { program.tick }
    assert_equal(2, program.scrtch.length )
    assert_equal(1, program.scrtch2.length )
    assert_equal(2, program.tbl.length )
  end
  
  def test_tuple_accessors
    program = BabyBloom.new('localhost', 12345)
    assert_nothing_raised( RuntimeError) { program.tick }
    assert_equal('a', program.scrtch.first.k1)
    assert_equal('b', program.scrtch.first.k2)
    assert_equal(1, program.scrtch[['a','b']].v1)
    assert_equal(2, program.scrtch[['a','b']].v2)
  end
  
  def test_insert_delete
    program = BabyBloom.new('localhost', 12345)
    # tick twice to get to 2nd timestep
    assert_nothing_raised( RuntimeError) { program.tick }
    assert_nothing_raised( RuntimeError) { program.tick }
    assert_equal(1, program.scrtch.length )
    assert_equal(0, program.scrtch2.length )
    assert_equal(2, program.tbl.length )
  end
  
  def test_dup_tables
    program = DupTableBloom.new('localhost', 12345)
    assert_raise( Bloom::BloomError ) { program.tick }
  end
  
  def test_dup_columns
    program = DupTableBloom.new('localhost', 12345)
    assert_raise( Bloom::BloomError ) { program.tick }
  end

  def test_dup_keys
    program = DupKeyBloom.new('localhost', 12345)
    assert_raise( Bloom::KeyConstraintError ) { program.tick }
  end
  
end
