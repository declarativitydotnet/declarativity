require "test/unit"

require 'lib/lang/parse/local_tw.rb'
require 'lib/lang/parse/tree_walker.rb'

require 'lib/lang/parse/schema.rb'

require 'lib/lang/plan/planner.rb'
require 'lib/types/table/object_table.rb'
require 'lib/lang/plan/predicate.rb'
#require 'lib/lang/compiler.rb'
require 'lib/lang/plan/program.rb'
require 'lib/lang/plan/rule.rb'
require 'lib/types/table/basic_table.rb'
require 'lib/types/table/catalog.rb'
require "lib/types/operator/scan_join"
require "lib/lang/plan/arbitrary_expression.rb"
require "lib/lang/plan/native_expression.rb"



class TestExpression < Test::Unit::TestCase
	def test_prog

		a = Variable.new("A",Integer)
		a.position = 0
		b = Variable.new("B",Integer)
		b.position = 1
		c = Variable.new("C",Integer)
		c.position = 2
		schema = Schema.new("me",[a,b,c])

		tup = Tuple.new(4,2,3)
		tup.schema = schema
		ax = ArbitraryExpression.new("((A + B) / C)",[a,b,c])
		foo = ax.function()
		assert_equal(2,foo.evaluate(tup))

		# the semantic effect should be the same as its more orderly, less powerful cousin:

		nx = NativeExpression.new("+",a,b)
		nx2 = NativeExpression.new("/",nx,c)
		f1 = nx2.function
		assert_equal(f1.evaluate(tup),foo.evaluate(tup))

		# binding an expressions over some variables, but leaving others free is no good
		badtup = Tuple.new(4,2)
		badschema = Schema.new("you",[a,b])
		badtup.schema = badschema
		assert_raise(RuntimeError) {foo.evaluate(badtup) } 
	

		
		ax = ArbitraryExpression.new("(A == 4) ? B : C",[a,b,c])

		func = ax.function

		assert_equal(2,func.evaluate(tup))
			
		tup2 = Tuple.new(1,2,3)
		tup2.schema = schema

		#puts func.evaluate(tup2)

		# the value of an expression over a constant damn well ought to be the value of that constant.
		ax = ArbitraryExpression.new("25",[a,b,c])
		func = ax.function
		assert_equal(25,func.evaluate(tup))

		
		
	end
	
end
