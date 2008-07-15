require "lib/types/basic/type_list"
require "test/unit"
require "rubygems"

class TestTypeList < Test::Unit::TestCase
  def default_test
    tl1 = TypeList.new([Integer,Float])
    assert(tl1==tl1)
    tl2 = TypeList.new([Integer])
    assert_not_equal(tl1==tl2,0)
    assert_not_equal(tl2==tl1,0)
    tl2 << String
    assert_equal(tl1<=>tl2,-1)
    tl3 = TypeList.new([Numeric,Numeric])
    assert_equal(tl3<=>tl1, 1)
    assert_equal(tl1<=>tl3, -1)
  end
end