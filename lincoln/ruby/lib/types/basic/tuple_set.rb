require "lib/types/table/table_name"

class TupleSet 
  include Comparable
  include Enumerable
  
  @@ids = 0
  def initialize(name, *tuples)
    @id = "TupleSet:" + @@ids.to_s
    @@ids += 1
    @tups = Array.new
    @name = name
    tuples.each {|t| @tups << t} unless tuples[0] == nil
  end
  
  def ts_id
    @id
  end
  
  attr_reader :tups, :name

  def size
    @tups.length
  end
  
  def to_s
    @id + super
  end
  
  def delete(tup)
    @tups.delete(tup)
  end
  
  def <<(tup)
    @tups << tup
  end
  
  def ==(o)
    if o.class == TupleSet
      return o.ts_id == @id
    else
      return false
    end
  end
  
  # The only meaningful response to this
  # method is to determine if the two sets
  # are equal.
  def <=>(o)
    return (self == o) ? 0 : 1
  end
  
  def each
    @tups.each do |t|
      yield t
    end
  end
end
  
  
  