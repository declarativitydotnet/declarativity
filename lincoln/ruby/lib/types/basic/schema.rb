require "lib/types/table/table_name"
require "lib/lang/plan/variable"
require "rubygems"
class Schema
  def initialize(name, vars)
    @name = name;
    @variables = Hash.new
    unless vars.nil?
      vars.each do |v|
        @variables[v.name] = v
      end 
    end
  end
  
  def set_schema(schema)
    @name = schema.name
    @variables = schema.variables.clone
  end

  def ==(s)
    return (@name == s.name && variables == s.variables)
  end
  
  attr_reader :name

  def size
    @variables.length
  end

  def <<(v)
    @variables[v.name] = v 
  end

  def contains(v)
    not @variables[v.name].nil?
  end

  def to_s
    out = "("
    @variables.sort{|a,b| a[1].position<=>b[1].position}.each do |a|
      out << a[1].to_s + ","
    end
    out[out.length-1] = ")"     if out.length > 1
    out << ")"       if out.length == 1
    out
  end

  def types
    out = Array.new
    @variables.sort{|a,b| a[1].position<=>b[1].position}.each do |a|
      out << a[1].expr_type  if a[1].position >= 0 
    end
    out
  end

  def variables
    out = Array.new
    @variables.sort{|a,b| a[1].position<=>b[1].position}.each do |a|
      out << a[1].clone if a[1].position >= 0 
    end
    out
  end

  def variable (name)
    @variables[name]
  end

  def schema_type(name)
    @variables[name].expr_type
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
