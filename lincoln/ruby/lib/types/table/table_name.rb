class TableName
  def initialize(scope, name)
    @scope = scope
    @name = name
  end
  def scope
    @scope
  end
  def name
    @name
  end
  def scope=(new_scope)
    @scope = new_scope
  end
  def name=(new_name)
    @name = new_name
  end
  def ==(o)
    return (o.class == TableName and o.to_s == to_s)
  end
  def to_s
    scope.to_s + "::" + name.to_s
  end
end
