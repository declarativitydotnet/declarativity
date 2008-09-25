require "lib/types/table/table_name"
require "lib/lang/plan/variable"
require "rubygems"
class Schema
  def initialize(name, vars)
    @name = name;
    @variable_set = Hash.new
    unless vars.nil?
      vars.each do |v|
        @variable_set[v.name] = v
      end 
    end
  end
    
  attr_accessor :variable_set
    
  def clone
    s = super
    s.variable_set = @variable_set.clone
    return s
  end
  
  def set_schema(schema)
    @name = schema.name
    @variable_set = schema.variables.clone
  end

  def ==(s)
    return (@name == s.name && variables == s.variables)
  end
  
  attr_reader :name

  def size
    @variable_set.length
  end

  def <<(v)
    @variable_set[v.name] = v 
  end

  def contains(v)
    not @variable_set[v.name].nil?
  end

  def to_s
     out = ""
#    out = "("
    i = 0
    @variable_set.sort{|a,b| a[1].position<=>b[1].position}.each do |a|
      out << "," if i > 0
      i += 1
      out << a[1].to_s
    end
 #   out[out.length-1] = ")"     if out.length > 1
#    out << ")"       if out.length == 1
    out
  end

  def types
    out = Array.new
    @variable_set.sort{|a,b| a[1].position<=>b[1].position}.each do |a|
      out << a[1].expr_type if a[1].position >= 0 
    end
    out
  end
  
  def variables
    out = Array.new
    @variable_set.sort{|a,b| a[1].position<=>b[1].position}.each do |a|
      out << a[1].clone if a[1].position >= 0 
    end
    out
  end

  def variable(name)
    @variable_set[name]
  end

  def schema_type(name)
    @variable_set[name].expr_type
  end

  def position(name)
    return (v=variable(name)).nil? ? nil : v.position
  end

  def join(inner)
    joined = Schema.new(nil, nil)
    self.variables.each do |v|
      joined << v
    end

    inner.variables.each do |v|
      unless joined.contains(v)
        joined << v
      end
    end
    joined
  end
end
