require "lib/types/table/table_name"

class UnsortedTupleSet 
  include Comparable
  include Enumerable
  
  @@ids = 0
  def initialize(name, *tuples)
    unless name.class <= TableName or name.class <= String
      raise "bad name for TupleSet"
    end
    @id = "TupleSet:" + @@ids.to_s
    @@ids += 1
#    @tups = Array.new
    @tups = Hash.new
    @name = name
#    tuples.each {|t| @tups << t} unless tuples[0] == nil
    tuples.each {|t| @tups[t.hash] = t} unless tuples[0] == nil
  end
  
  def ts_id
    @id
  end
  
  attr_reader :name
  
  def tups
    return @tups.values
  end

  def size
    @tups.length
  end
  
  def to_s
    @id ###+ super
  end
  
  def hash
    @id.hash
  end
  
  def delete(tup)
    @tups.delete(tup.hash)
  end
  
  def clear
#    @tups = Array.new
    @tups = Hash.new
  end
  
  def UnsortedTupleSet=(ts)
    return self.class.new(ts.name, *ts.tups)
  end
  
  def +(ts)
    retval = self.class.new(name, *tups)
    ts.each {|t| retval << t}
    return retval
  end
  
  def <<(o)
    case o.class.name
      #      when 'TupleSet': o.each {|t| @tups << t }
    when 'TupleSet', 'Array': 
      require 'ruby-debug'; debugger
      raise "using << rather than addAll to add a collection into a TupleSet"
    when 'Tuple':
      @tups[o.hash] = o
    else 
      puts o.inspect	
      raise "inserting a " + o.class.name + " object into TupleSet"
    end
  end
  
  def addAll(ts)
    ts.each {|t| self << t} unless ts.nil?
    return self
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

  def ordered_array(*col_names)
    out = @tups.values.sort do |a,b| 
      diff = 0
      col_names.each do |c|
        ac = a.value(c)
        bc = b.value(c)
        raise("can't order by unknown column set #{c}") if (ac.nil? or bc.nil?)
        diff = (ac <=> bc)
        break unless diff == 0
      end
      diff
    end
    return out
  end  
  
  def order_by(*col_names)
    ordered_array(*col_names).each  {|t| yield t}
  end
  
  def each
    @tups.values.each do |t|
      yield t
    end
  end
end

class TupleSet < UnsortedTupleSet
  def initialize(name, *tuples)
    # this ordered list will point to hash keys in the superclass
    @positions = Array.new
    @back_ptrs = Hash.new
    super(name,nil)
    unless tuples[0] == nil
      tuples.each { |t| self << t }
    end
  end

  def delete(tup)
	res = super(tup)
	if res == tup
	  pos = @back_ptrs[tup.hash]
      @positions.delete_at(pos)
      pos.upto(@positions.length - 1) do |i|
        unless @back_ptrs[@positions[i]] == i+1
          require 'ruby-debug'; debugger
          raise "tupleset bug" 
	    end
        @back_ptrs[@positions[i]] = i
	  end
	end
	return res
  end

  def clear
    super
    @back_ptrs = Hash.new
    @positions = Array.new
  end

  # def TupleSet=(ts)
  #   return TupleSet.new(ts.name, ts.tups)
  # end
  # 
  # 
  def <<(o)
    case o.class.name
    when 'TupleSet','Array': o.each do |t| 
      require 'ruby-debug';debugger
      raise "using << rather than addAll to add a collection into a TupleSet"
    end
    when 'Tuple':
      @tups[o.hash] = o
      @positions << o.hash
      @back_ptrs[o.hash] = @positions.length-1
      super(o)
    else 
 #     puts o.inspect	
      raise "inserting a " + o.class.name + " object into TupleSet"
    end
  end

  def tups
    out = Array.new
    each {|t| out << t}
    return out
  end
  
  def each # return them in order of position!
    #raise "uh oh tupleset!" unless @positions.length == @tups.length
    #raise "uh oh tupleset!" unless @positions.length == @tup_pos.length
    @positions.each do |p|
      yield @tups[p] unless @tups[p].nil?
    end
  end
end
