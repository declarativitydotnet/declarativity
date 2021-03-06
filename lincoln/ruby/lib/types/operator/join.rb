require 'lib/types/operator/operator'
require 'lib/types/function/tuple_function'
require 'lib/types/basic/tuple'

class Join < Operator 

  attr_reader :schema
  
  class TableField < TupleFunction
    include Comparable

    def initialize(type, position)
      @type = type
      @position=position
    end

    def evaluate(tuple) 
      return tuple.values[@position];
    end

    def returnType
      @type;
    end
  end

  class JoinFilter 
    def initialize(lhs, rhs)
      @lhs = lhs;
      @rhs = rhs;
    end

    attr_reader :lhs, :rhs
    
    def evaluate(outer, inner)
      if (@lhs.class <= TableField)
        lvalue = @lhs.evaluate(inner)
      else 
        lvalue = @lhs.evaluate(outer) 
      end

      if (@rhs.class <= TableField) 
        rvalue = @rhs.evaluate(inner)
      else 
        rvalue = @rhs.evaluate(outer)
      end
#      # require 'ruby-debug'; debugger if lvalue == true
      return (lvalue == rvalue)
    end
  end

  def initialize(context, predicate, input) 
    super(context, predicate.program, predicate.rule)
    @predicate = predicate
    @filters = filters(predicate)
    @schema = input.join(predicate.schema)
  end

  def requires
    @predicate.requires
  end

  def validate(outer, inner)
    @filters.each do |f|
      return false if (f.evaluate(outer, inner) == false)
    end	
    return true
  end

  def filters(predicate)
    filters = Array.new

    positions = Hash.new
    predicate.each do |arg|
      if (arg.class <= Variable) then
        var = arg
        if (positions[var.name])
          prev = positions[var.name]
          filters << JoinFilter.new(TableField::new(prev.expr_type, prev.position), TableField::new(var.expr_type, prev.position))
        else 
          positions[var.name] = var
        end
      else 
        filters << JoinFilter.new(TableField::new(arg.class, arg.position), arg.function)
      end
    end
#    # require 'ruby-debug'; debugger
#    raise "Join filters not resolved" if (positions != {} && filters == [])
    return filters;
  end
end
