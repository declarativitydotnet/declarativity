require "lib/types/table/index.rb"

class HashIndex < Index
  attr_reader :map
  
  def initialize(table, key, type)
    super(table, key, type)
    @map = Hash.new
    table.tuples.map {|t| insert(t)}
  end
  
  def to_s
    out = "Index " + table.name + "\n"
    unless @map.nil?
      out += @map.to_s + "\n"
    end
    return out
  end
  
  def clear
    @map = Hash.new
  end
  
  def insert(t)
    k = @key.project(t).values
    if @map.has_key?(k)
      @map[k] << t
    else
      tuples = TupleSet.new(table.name, nil)
      tuples << t
      @map[k] = tuples
    end
  end
  
  def lookup(t)
    k = @key.project(t).values
    return @map.has_key?(k) ? @map[k] : TupleSet.new(table.name, nil)
  end
  
  def lookup_kt(the_key, t)
    k = the_key.project(t).values
    @map.has_key?(k) ? @map[k] : TupleSet.new(table.name, nil)
  end
  
  def lookup_vals(*values)
    if (values.length != @key.size)
      throw "Value length does not match key size!"
    end
    
    keyValues = Array.new
    values.each {|v| keyValues << v}
    the_key = Tuple.new(*keyValues)
    return @map[the_key.values]
  end
  
  def delete(t)
    k = @key.project(t).values
    if @map.has_key?(k) then
      @map[k].delete(t)
    end
  end
  
  
end