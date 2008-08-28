require "test/unit"

require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'

require 'lib/lang/parse/schema.rb'

require 'lib/lang/plan/planner.rb'
require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
#require 'lib/lang/@rb'
require 'lib/lang/plan/program.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"
require "lib/lang/plan/arbitrary_expression.rb"

require 'lib/lang/parse/procedural.rb'


class TestPlan < Test::Unit::TestCase
	
	def test_gen_link_tuples
	  # we need to manually construct a schema spec; we can't infer it from link (materialized by the overlog above) 
		# because link is empty, and schema belong to tuples, not tables (not that link's fields have no names)
		t1 = Tuple.new("N1","N2",10,"first")
		t2 = Tuple.new("N2","N3",5,"second")

		# ... from test_program
		v1 = Variable.new("From", String)
		v1.position = 0
		v2 = Variable.new("To", String)
		v2.position = 1
		v3 = Variable.new("Cost", Float)
		v3.position = 2
		v4 = Variable.new("Annotation", String)
		v4.position = 3   

		schema1 = Schema.new("schema1", [v1,v2,v3,v4])
		t1.schema = schema1
		t2.schema = schema1
		tn = TableName.new(nil, "link")
		assert_equal(1,1)
		return tn, TupleSet.new(tn, t1, t2)		
	end
	


end
