class Expression
  attr_accessor :position, :location

  def to_s
    throw "Abstract method Expression.to_s not subclassed"
  end
    
  def variables
    throw "Abstract method Expression.variables not subclassed"
  end
    
  def function
    throw "Abstract method Expression.to_s not subclassed"
  end
end