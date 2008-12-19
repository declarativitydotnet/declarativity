
require 'lib/lang/plan/variable'

class Aggregate < Variable 
	@@STAR = '*'
	
	def initialize(name, function, the_type, position)
#	  require 'ruby-debug'; debugger
		super(name, the_type, position, nil)
		@function = function
	end
	
	def clone
		Aggregate.new(@name, @function, expr_type, position)
	end

	def to_s
		@function.to_s + "<" + super + ">"
	end
	
	def functionName
		@function
	end
	
  def function
    # Ruby MetaProgramming-Fu!
    # the lambda's here make sure that the local state, i.e. @value, is 
    # in a closure, so when these functions are called, they'll remember 
    # that state
    e_lam = lambda do |t|
      return name == @@STAR ? t.id : t.value(name)
    end

    tmpClass = Class.new(TupleFunction)
    tmpClass.send :define_method, :evaluate do |tuple|
      e_lam.call(tuple)
    end
    return tmpClass.new
  end
end

