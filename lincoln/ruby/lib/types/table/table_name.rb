class TableName
  include Comparable
  
  def initialize(scope, name)
    @scope = scope
    @name = name
  end
  
  attr_accessor :scope, :name

  def ==(o)
    return (o.class == TableName and o.scope == scope and o.name == name)
  end

  def hash
    return to_s.hash
  end
  
  def eql?(o)
    o.is_a? TableName && to_s == o.to_s
  end
  
  def to_s
    scope.to_s + "::" + name.to_s
  end
  
  def <=>(tn)
    return (to_s <=> tn.to_s)
  end
end
