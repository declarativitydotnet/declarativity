class TableName
  def initialize(scope, name)
    @scope = scope
    @name = name
  end
  
  attr_accessor :scope, :name
  def ==(o)
    return (o.class == TableName and o.to_s == to_s)
  end
  def to_s
    scope.to_s + "::" + name.to_s
  end
end
