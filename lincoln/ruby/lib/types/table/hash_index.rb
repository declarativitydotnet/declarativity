require "lib/types/table/index.rb"

class HashIndex < Index
  attr_reader :map
  
  def initialize(context, table, key, type)
    super(context, table, key, type)
    @map = Hash.new
    ##table.tuples.each {|t| insert(t);  print "table #{table}, inserting #{t}\n";}
#    # require 'ruby-debug'; debugger
    table.tuples.each {|t| insert(t)  }
  end
  
  def to_s
    out = "Index " + table.name.to_s + "\n"
    unless @map.nil?
      out += @map.to_s + "\n"
    end
    return out
  end
  
  def clear
    @map.clear
  end
  
  def insert(t)
    k = @key.project(t).values.hash
    if @map.has_key?(k)
      @map[k] << t
    else
      tuples = TupleSet.new(table.name, nil)
      tuples << t
      @map[k] = tuples
    end
  end
  
  def lookupByKey(the_key)
		if (@key.size != the_key.size and @key.size > 0) 
			raise "Key for #{table.name} with wrong number of columns.  " + "Saw: " + the_key.size.to_s + " expected: " + @key.size.to_s + " key: " + the_key.to_s
		end
		the_key = the_key.hash
		return (@map.has_key?(the_key)) ? @map[the_key] : TupleSet.new(table.name)
	end
	
  def lookup(t)
    k = @key.project(t).values.hash
    return @map.has_key?(k) ? @map[k] : TupleSet.new(table.name, nil)
  end
  
  def lookup_kt(the_key, t)
    k = the_key.project(t).values.hash
	#print "LOOKUP #{the_key.project(t)} (hashes to #{k})\n"
	#print "__________________\nWITHING #{@map.keys.join(",")}\n"
#$	#print "AKA 
    @map.has_key?(k) ? @map[k] : TupleSet.new(table.name, nil)
  end
  
  def lookup_vals(*values)
    if values.length != @key.size
      throw "Value length does not match key size!"
    end
    
    keyValues = Array.new
    values.each {|v| keyValues << v}
    the_key = Tuple.new(*keyValues)
    return @map[the_key.values.hash]
  end
  
  def delete(t)
    k = @key.project(t).values.hash
    if @map.has_key?(k)
      @map[k].delete(t)
    end
  end
end
