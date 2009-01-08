
require 'lib/lang/plan/variable'

class Aggregate < Variable 
	@@STAR = '*'
	
	def initialize(name, function, position)
#	  # require 'ruby-debug'; debugger
    the_type = AggregateFunction.agg_type(function, String) 
		super(name, the_type, position, nil)
		@function = function
		@method = AggregateFunction.function(self).accessor
    raise "aggregate function #{@aggfunc} unrecognized" if @method.nil?
	end
	
	def clone
		Aggregate.new(@name, @function, position)
	end

	def to_s
		@function.to_s + "<" + super + ">"
	end
	
	def functionName
		@function
	end
	
  def function
    return @method unless @method.nil?
    
    # otherwise synthesize a method to return last tuple value
    
    # Ruby MetaProgramming-Fu!
    # the lambda's here make sure that the local state, i.e. @value, is 
    # in a closure, so when these functions are called, they'll remember 
    # that state
    e_lam = lambda do |t|
      return name == @@STAR ? t.id : t.name_value(name)
    end

    tmpClass = Class.new(TupleFunction)
    tmpClass.send :define_method, :evaluate do |tuple|
      e_lam.call(tuple)
    end
    return tmpClass.new
  end
end

