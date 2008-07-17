require "lib/lang/plan/expression"
class Variable < Expression
  def initialize(name, type)
    @name = name
    @type = type
    @position = -1
  end 

  def ==(o)
    if o.class == Variable then
      return (o.name == name)
    end
  end

  def clone
    v = Variable.new(name,type)
    v.position = position
    return v
  end
  
  attr_reader :name
  attr_accessor :type

  def to_s
    position >= 0 ? name + ":" + position.to_s : name
  end

  def variables
    v = Array.new
    v << self
    return v
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
      return @type
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

