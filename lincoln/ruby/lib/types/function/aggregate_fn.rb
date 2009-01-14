require 'lib/types/basic/tuple_set'
require 'lib/types/function/tuple_function'
require 'lib/lang/plan/aggregate'

class AggregateFunction < TupleFunction	  
  class Accessor < TupleFunction
    def initialize(aggregate)
      @position = aggregate.position
      @type     = aggregate.class
      @name     = aggregate.name
      @in_pos   = nil
    end

    attr_reader :position, :in_pos
    
    def evaluate(tuple)
      #      # require 'ruby-debug'; debugger
#      @in_pos = tuple.name_pos(@name) if @in_pos.nil?
#      tuple.int_value(@in_pos)
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
      @result = nil
      reset
    end
    
    attr_reader :accessor
    attr_reader :result
    
    def reset
      @result = nil
    end
    
    def evaluate(tuple)
#      require 'ruby-debug'; debugger
      insert(tuple)
      return result
    end
  end

  class Exemplary < StdAgg
    def initialize(accessor)
      super(accessor)
      reset
    end

    def reset
      super
      @result = nil
      @current = nil
      @tuples = TupleSet.new("agg")
    end

    def tuples
      @tuples.clone
    end

    attr_reader :result

    def insert(tuple)
      if (@tuples << tuple)
        value = @accessor.evaluate(tuple)
        if (@current.nil? || prefer_new(@result, value)) # > 0
          @current = value
          @result = tuple
        end
      end
    end

    # def evaluate(tuple)
    #   insert(tuple)
    #   return @result
    # end
    # 
    def delete(tuple)
      if @tuples.remove(tuple)
        value = @accessor.evaluate(tuple)
        if (@current == value)
          tuples = @tuples
          reset
          tuples.each {|t| insert(t)}
        end
      end
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
      newer < current ? true : false
    end
  end


  class Max < Exemplary	
    def prefer_new(current, newer)
      newer > current ? true : false
    end
  end


  class Count	< StdAgg
    def initialize(accessor)
      super(accessor)
      reset
    end
    
    def reset
      @cnt = 0
      @result = nil
    end

    def result
      unless @result.nil?
        @result = result.clone
        @result.set_value(@accessor.position, size)
      else
        nil
      end
    end

    def insert(tuple)
      @cnt += 1
    end

    def delete(tuple)
      @cnt -= 1 if @cnt > 0
    end

    def size
      @cnt
    end

    def returnType
      Integer
    end
  end

  class Avg < StdAgg
    def initialize(accessor)
      super(accessor)
      reset
    end
    
    def reset
      @result = nil
      @sum = 0
      @cnt = 0
    end

    def result
      return nil if @result.nil?
      @result = @result.clone
      @result.set_value(@accessor.position, (1.0 * @sum) / (1.0 * @cnt))
    end

    def insert(tuple)
      @sum += @accessor.evaluate(tuple) 
      @cnt += 1
    end

    def delete(tuple)
      if @cnt > 0
        @sum -= @accessor.evaluate(tuple) 
        @cnt -= 1
      end
    end

    def size
      @cnt
    end

    def returnType
      Float
    end
  end
  
  class Sum < StdAgg
    def initialize(accessor)
      super(accessor)
      reset
    end
    
    def reset
      @result = nil
      @sum = 0
      @cnt = 0
    end

    def result
      return nil if @result.nil?
      @result = @result.clone
      @result.set_value(@accessor.position, @sum)
    end

    def insert(tuple)
      @sum += @accessor.evaluate(tuple) 
      @cnt += 1
    end

    def delete(tuple)
      if @cnt > 0
        @sum -= @accessor.evaluate(tuple) 
        @cnt -= 1
      end
    end

    def size
      @cnt
    end

    def returnType
      Float
    end
  end


  class ConcatString < StdAgg
    def initialize(accessor)
      super(accessor)
      reset
    end

    def result
      if @result.nil?
        @result = @result.clone
        @result.set_value(@accessor.position, @current)
        return @result
      end
      return nil
    end

    def reset 
      super
      @tuples = TupleSet.new("ConcatStringAgg")
      @result = nil
      @current = nil
    end

    def insert(tuple)
      if (@tuples << tuple)
        @result = tuple
        if @result.nil?
          @current = @accessor.evaluate(tuple)
        else 
          @current += @accessor.evaluate(tuple)
        end
        return result
      end
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
      reset
    end
    
    def reset
      @tuples = TupleSet.new("TupleSetAgg")
      @nestedSet = TupleSet.new("NestedTupleSetAgg")
      @the_result = nil
    end

    def result
      @the_result = @the_result.clone
      @the_result.set_value(accessor.position, @nestedSet.clone)
      return @the_result
    end

    def insert(tuple)
      unless (@tuples << tuple).nil?
        @the_result = tuple
        @nestedSet << @accessor.evaluate(tuple) 
        return result
      end
      return nil
    end

    def delete(tuple)
      if (@tuples.remove(tuple)) 
        @the_result = tuple
        @nestedSet.remove(@accessor.evaluate(tuple)) 
        return result
      end
    end

    def size
      @tuples.size
    end
    
    # def evaluate(tuple)
    #   insert(tuple)
    #   return result
    # end
    # 
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
#       accessor.evaluate(@result) if accessor.in_pos.nil?
#       @result.set_value(accessor.in_pos, @current)
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
#       accessor.evaluate(@result) if accessor.in_pos.nil?
#       @result.set_value(accessor.in_pos, @tupleset)
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
