require 'lib/types/operator/operator'
require 'lib/types/function/tuple_function'
require 'lib/types/basic/tuple'

class Join < Operator 

  class TableField < TupleFunction
    include Comparable

    def initialize(type, position)
      @type = type
      @position=position
    end

    def evaluate(tuple) 
      return tuple.value(@position);
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

    def lhs
      @lhs
    end
    
    def rhs
      @rhs
    end
    
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
      return ((lvalue <=> rvalue) == 0)
    end
  end

  def initialize(predicate, input) 
    super(predicate.program, predicate.rule)
    @predicate = predicate
    @filters = filters(predicate)
    @schema = input.join(predicate.schema)
  end

  def schema
    @schema;
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
          filters << JoinFilter.new(TableField.new(prev.type, prev.position), TableField.new(var.type, prev.position))
        else 
          positions[var.name] = var
        end
      else 
        filters << JoinFilter.new(TableField.new(arg.class, arg.position), arg.function)
      end
    end
#    require 'ruby-debug'; debugger
#    raise "Join filters not resolved" if (positions != {} && filters == [])
    return filters;
  end
end
