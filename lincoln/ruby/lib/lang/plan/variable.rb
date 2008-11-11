require "lib/lang/plan/expression"

class Variable < Expression
  def initialize(name, type, position, loc)
    @name = name
    @expr_type = type
    @position = position
    @loc = loc
  end

  def ==(o)
    if o.class == Variable
      return o.name == @name
    end
  end

  def clone
    Variable.new(@name, @expr_type, @position, @loc)
  end
  
  attr_reader :name
  attr_accessor :expr_type, :loc

  def hash
    @name.hash
  end
  
  def to_s
    @position >= 0 ? @name + ":" + @position.to_s : @name
  end

  def variables
    [self]
  end

  def function
    # Ruby MetaProgramming-Fu!
    # the lambda's here make sure that the local state, i.e. @value, is 
    # in a closure, so when these functions are called, they'll remember 
    # that state
    e_lam = lambda do |t|
      return t.value(@name)
    end

    r_lam = lambda do
      return @expr_type
    end

    tmpClass = Class.new(TupleFunction)
    tmpClass.send :define_method, :evaluate do |tuple|
      e_lam.call(tuple)
    end
    tmpClass.send :define_method, :returnType do 
      r_lam.call
    end
    return tmpClass.new
  end
end

