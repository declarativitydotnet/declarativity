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
      tuple.value(@name)
    end

    def returnType
      @type;
    end
  end


  @@MIN      = "min"
  @@MAX      = "max"
  @@COUNT    = "count"
  @@AVG      = "avg"
  @@SUMSTR   = "sumstr"
  @@MAKESET  = "tupleset"


  def AggregateFunction.function(aggregate)
    retval = case aggregate.functionName
    when @@MIN: Min.new(Accessor.new(aggregate))
    when @@MAX: Max.new(Accessor.new(aggregate))
    when @@COUNT: Count.new(Accessor.new(aggregate))
    when @@AVG: Avg.new(Accessor.new(aggregate))
    when @@SUMSTR: SumStr.new(Accessor.new(aggregate))
    when @@MAKESET: MakeSet.new(Accessor.new(aggregate))
    else nil
    end

    return retval
  end

  def AggregateFunction.agg_type(function, type) 
    retval = case function
    when @@MIN: type
    when @@MAX: type
    when @@COUNT: Integer
    when @@AVG: Float
    when @@SUMSTR: String
    when @@MAKESET: TupleSet
    else nil
    end
  end

  class Exemplary
    def initialize(accessor)
      @result = nil
      @current = nil
      @accessor = accessor
    end

    def result
      @result.nil?  ? nil : @result.clone
    end

    def reset
      @result = nil
      @current = nll
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

    def prefer_new(current, newer)
      raise "exemplary superclass prefer_new not supported"
    end

    def returnType
      @accessor.returnType
    end
  end

  class Min < Exemplary
    def prefer_new(current, newer)
      current < newer ? true : false
    end
  end


  class Max < Exemplary	
    def prefer_new(current, newer)
      current > newer ? true : false
    end
  end


  class Count	
    def initialize(accessor)
      @result = nil
      @current = 0
      @accessor = accessor
    end

    def result
      return nil if @result.nil?
      @result = @result.clone()
      @result.value(accessor.position, @current)
      return @result
    end

    def reset
      @current = 0
    end

    def evaluate(tuple)
      @result = tuple
      @current += 1
      return @current
    end

    def returnType
      Integer
    end
  end

  class Avg
    def initialize(accessor)
      @result = nil
      @sum = @count = 0
      @accessor = accessor
    end

    def result
      return nil if @result.nil
      @result = @result.clone();
      @result.value(accessor.position, @sum / @count)
      return @result
    end

    def reset
      @result = nil
      @sum = 0
      @count = 0
    end

    def evaluate(tuple)
      @result = tuple
      @sum += @accessor.evaluate(tuple).to_f*1.0
      @count += 1.0
      return @sum / @count
    end

    def returnType
      Float
    end
  end

  class SumStr 
    def initialize(accessor)
      @current = nil
      @accessor = accessor
    end

    def result
      return nil if @result.nil? 
      @result = @result.clone()
      @result.value(@accessor.position, @current)
      return @result;
    end

    def reset
      @result = nil
      @current = nil
    end

    def evaluate(tuple)
      @result = tuple
      if @current.nil?
        @current = @accessor.evaluate(tuple) 
      else
        @current += @accessor.evaluate(tuple) 
      end
      return @current
    end

    def returnType
      String
    end
  end

  class MakeSet
    def initialize(accessor)
      @result = nil
      @tupleset = nil
      @accessor = accessor
    end

    def result
      return nil if @result.nil?
      @result = @result.clone()
      @result.value(@accessor.position, @tupleset)
      return @result
    end

    def reset
      @result = nil
      @tupleset = nil
    end

    def evaluate(tuple)
      @result = tuple
      @tupleset = TupleSet.new("agg") if @tupleset.nil?
      @tupleset << @accessor.evaluate(tuple)
      return @tupleset
    end

    def returnType
      TupleSet
    end
  end
end
