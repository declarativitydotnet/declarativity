require 'lib/types/basic/tuple_set'
require 'lib/types/function/tuple_function'
require 'lib/lang/plan/aggregate'

class AggregateFunction < TupleFunction	  
  class Accessor < TupleFunction
    def initialize(aggregate)
      @position = aggregate.position
      @type     = aggregate.class
      @name     = aggregate.name
    end

    attr_reader :position

    def evaluate(tuple)
      #      # require 'ruby-debug'; debugger
      tuple.name_value(@name)
    end

    def returnType
      @type
    end
  end


  @@MIN      = "min"
  @@MAX      = "max"
  @@COUNT    = "count"
  @@AVG      = "avg"
  @@SUMSTR   = "sumstr"
  @@TUPLESET  = "tupleset"


  def AggregateFunction.function(aggregate)
    retval = case aggregate.functionName
    when @@MIN: Min.new(Accessor.new(aggregate))
    when @@MAX: Max.new(Accessor.new(aggregate))
    when @@COUNT: Count.new(Accessor.new(aggregate))
    when @@AVG: Avg.new(Accessor.new(aggregate))
    when @@SUMSTR: SumStr.new(Accessor.new(aggregate))
    when @@TUPLESET: TupleCollection.new(Accessor.new(aggregate))
    else nil
    end

    return retval
  end

  def AggregateFunction.agg_type(function, type) 
    retval = case function
      # when @@MIN: type
      # when @@MAX: type
    when @@MIN: Integer
    when @@MAX: Integer
    when @@COUNT: Integer
    when @@AVG: Float
    when @@SUMSTR: String
    when @@TUPLESET: TupleSet
    else nil
    end
  end

  class StdAgg
    def initialize(accessor)
      @accessor = accessor
      reset
    end
    
    attr_reader :accessor
    
    def reset
      @result = nil
    end
  end

  class Exemplary < StdAgg
    def initialize(accessor)
      super(accessor)
      reset
    end

    def reset
      super
      @current = nil
      @tuples = TupleSet.new("agg")
    end

    def tuples
      @tuples.clone
    end

    def result
      @result.nil?  ? nil : @result.clone
    end

    def insert(tuple)
      if (@tuples << tuple)
        return evaluate(tuple)
      end
    end

    def evaluate(tuple)
      value = @accessor.evaluate(tuple)
      if (@current.nil? || prefer_new(@current, value)) # > 0
        @current = value
        @result = tuple
        return value
      end
      return nil
    end

    def delete(tuple)
      if @tuples.remove(tuple)
        value = @accessor.evaluate(tuple)
        if (@current == value)
          tuples = @tuples
          reset
          tuples.each {|t| insert(t)}
          return result
        end
      end
      return nil
    end

    def prefer_new(current, newer)
      raise "exemplary superclass prefer_new not supported"
    end

    def returnType
      @accessor.returnType
    end
  end

  class Min < Exemplary
    def prefer_new(current, newer)
      # require 'ruby-debug'; debugger
      current < newer ? true : false
    end
  end


  class Max < Exemplary	
    def prefer_new(current, newer)
      current > newer ? true : false
    end
  end


  class Count	< StdAgg
    def initialize(accessor)
      super(accessor)
      @tuples = TupleSet.new("agg")
    end

    def result
      return nil if @result.nil?
      @result = @result.clone
      @result.set_value(accessor.position, @tuples.size)
      return @result
    end

    def insert(tuple)
      if (@tuples << tuple)
        @result = tuple
        return result
      end
      return nil
    end

    def delete(tuple)
      @result = tuple
      @tuples.remove(tuple)
      return result
    end

    def returnType
      Integer
    end

    def tuples
      @tuples.clone
    end
  end

  class Avg < StdAgg
    def initialize(accessor)
      super(accessor)
      reset
    end

    def result
      return nil if @result.nil
      @result = @result.clone
      @result.set_value(accessor.position, @sum / @tuples.size)
      return @result
    end

    def reset
      super
      @tuples = TupleSet.new("agg")
      @sum = 0
    end

    def insert(tuple)
      if (tuples << tuple)
        @result = tuple
        value = @accessor.evaluate(tuple)
        @sum += value.to_f
        return result
      end
      return nil
    end

    def delete(tuple)
      if (@tuples.remove(tuple))
        @tuples << tuple
        @result = tuple
        value = @accessor.evaluate(tuple)
        @sum -= value.to_f
        return result
      end
      return nil
    end

    def returnType
      Float
    end

    def tuples
      @tuples.clone
    end
  end

  class ConcatString < StdAgg
    def initialize(accessor)
      super(accessor)
      reset
    end

    def result
      unless (@result.nil?) 
        @result = @result.clone
        @result.set_value(accessor.position, @current)
        return @result
      end
      return nil
    end

    def reset 
      super
      @tuples = TupleSet.new
      @current = nil
    end

    def insert(tuple)
      if (@tuples << tuple)
        @result = tuple
        if @current.nil?
          @current = @accessor.evaluate(tuple)
        else 
          @current += @accessor.evaluate(tuple)
        end
        return result
      end
      return nil
    end

    def delete(tuple)
      if (@tuples.remove(tuple)) 
        tuples = @tuples
        reset
        tuples.each { |copy| insert copy }
        return result
      end
      return nil
    end

    def returnType 
      String
    end

    def tuples 
      return @tuples.clone
    end
  end

  class TupleCollection < StdAgg
    def initialize(accessor) 
      super(accessor)
      @tuples = TupleSet.new("agg")
      @nestedSet = TupleSet.new("agg")
    end

    def result 
      return @result if @resul.nil?

      @result = @result.clone
      @result.set_value(@accessor.position, @nestedSet.clone)
      return @result
    end

    def insert(tuple)
      if (@tuples << tuple) 
        @result = tuple
        @nestedSet << @accessor.evaluate(tuple)
        return result
      end
      return nil
    end

    def delete(tuple)
      if (@tuples.remove(tuple)) 
        @result = tuple
        @nestedSet.remove(@accessor.evaluate(tuple))
        return result
      end
      return nil
    end

    def returnType 
      TupleSet
    end

    def tuples 
      @tuples.clone
    end
  end
end

#   class SumStr 
#     def initialize(accessor)
#       @current = nil
#       @accessor = accessor
#     end
# 
#     def result
#       return nil if @result.nil? 
#       @result = @result.clone
#       @result.set_value(@accessor.position, @current)
#       return @result;
#     end
# 
#     def reset
#       @result = nil
#       @current = nil
#     end
# 
#     def evaluate(tuple)
#       @result = tuple
#       if @current.nil?
#         @current = @accessor.evaluate(tuple) 
#       else
#         @current += @accessor.evaluate(tuple) 
#       end
#       return @current
#     end
# 
#     def returnType
#       String
#     end
#   end
# 
#   class MakeSet
#     def initialize(accessor)
#       @result = nil
#       @tupleset = nil
#       @accessor = accessor
#     end
# 
#     def result
#       return nil if @result.nil?
#       @result = @result.clone
#       @result.set_value(@accessor.position, @tupleset)
#       return @result
#     end
# 
#     def reset
#       @result = nil
#       @tupleset = nil
#     end
# 
#     def evaluate(tuple)
#       @result = tuple
#       @tupleset = TupleSet.new("agg") if @tupleset.nil?
#       @tupleset << @accessor.evaluate(tuple)
#       return @tupleset
#     end
# 
#     def returnType
#       TupleSet
#     end
#   end
# end
