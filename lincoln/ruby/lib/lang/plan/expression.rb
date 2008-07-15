class Expression
  def position
    @position
  end
  def position=(i)
    @position = i
  end
  
  
  def location=(l) 
    @location = l
  end
  
  def location() 
    @location
  end

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