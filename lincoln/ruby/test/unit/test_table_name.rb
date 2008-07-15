require "test/unit"
require "lib/types/table/table_name"
class TestTableName < Test::Unit::TestCase
  def test_simple
    t = TableName.new("global", "t1")
    t2 = t.clone
    assert(t == t2)
    assert(t.to_s == "global::t1")
    assert_equal(t.scope, "global")
    assert_equal(t.name, "t1")
    t.scope = "parser"
    t.name="t2"
    assert_equal(t.scope, "parser")
    assert_equal(t.name, "t2")
  end
end