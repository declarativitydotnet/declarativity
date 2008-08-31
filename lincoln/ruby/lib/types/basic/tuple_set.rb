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
  
  def hash
    @id.hash
  end
  
  def delete(tup)
    @tups.delete(tup)
  end
  
  def clear
    @tups = Array.new
  end
  
  def <<(o)
    case o.class.name
      when 'TupleSet': o.each {|t| @tups << t }
      when 'Tuple': @tups << o
      else raise "inserting a " + o.class.name + " object into TupleSet"
    end
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

  def order_by(*col_names)
     @tups.sort { |a,b| 
		retval = last = nil
		col_names.each do |c|
			ac = a.value(c)
			bc = b.value(c)
			raise("can't order by unknown column set #{c}") if ac.nil?
			if ((ac <=> bc) != 0) then
				retval = (ac <=> bc)
				break;
			end
			last = c
		end

		# fallthrough
		if retval.nil? then	
			retval =  a.value(last) <=> b.value(last)
		end
		retval
	}.each do |t|
        yield t
     end
  end  
  def each
    @tups.each do |t|
      yield t
    end
  end
end
  
  
  
