class TypeList
  include Comparable
  
  def initialize(types)
    @types = types
  end
  
  def ==(o)
    if (o.class == TypeList) and ((self <=> o) == 0)
      return true
    else
      return false
    end
  end
  
  def size
    return @types.length
  end
  
  def [](i)
    return @types[i]
  end
  
  def <<(t)
    @types << t
    return self
  end
  
  def <=>(o)
    if size < o.size
      return -1
    elsif size > o.size
      return 1
    else
      (0..(size-1)).each do |i|
        if self[i].hash < o[i].hash
          return -1
        elsif self[i].hash > o[i].hash
          return 1
        end
      end
      return 0
    end
  end
end