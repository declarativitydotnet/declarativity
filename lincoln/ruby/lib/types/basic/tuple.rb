require 'lib/types/basic/schema'
require "rubygems"

class Tuple
  include Comparable
  
  @@idGen = 0
  def initialize(*values)
    init
    @values = Array.new
    values.each do |v|
      @values << v
    end
  end

  def init
    @schema = Schema.new(nil,nil)
    @count = 1
    @tid = @@idGen.to_s
    @@idGen += 1
  end
  
  attr_accessor :tid, :count
  attr_reader :schema, :values

  def append (var, val)
    var = var.clone
    var.position = schema.size
    @schema << var
    @values << val
  end

  def to_s
    unless values.length <= 0
      out = "<";
      values.each do |v|
        out << (v.nil? ? "null" : v.to_s) + ", "
      end
      out[out.length - 2] = ">"
      out.rstrip
    end
  end

  def schema=(s)
    if s.size != size then
#      require 'ruby-debug'; debugger
      raise "Schema assignment does not match tuple arity!  schema " + s.to_s
    end
    @schema = s
  end
  
  def <=>(o)
    if (@values.size == 0)
      return (tid <=> o.tid)
    else
      if size != o.size then
        return -1
      end
      (0..(@values.size-1)).each do |i|
        if ((@values[i].nil? || o.values[i].nil?)) then 
          if @values[i] != o.values[i] then
            return -1
          end
        elsif (@values[i] <=> o.values[i]) != 0 then
          return @values[i] <=> o.values[i]
        end
      end
    end
    return 0
  end

  def ==(o)
    (o.class == Tuple) && ((o <=> self) == 0);
  end
  
  def size() 
    return @values.length
  end
  
  def value(i)
    if i.class <= Numeric
      return values[i]
    else
      return values[@schema.position(i)]
    end
  end  
  
  def set_value(i, value)
    if i.class <= Numeric
      @values[i] = value
    elsif i.class == Variable
      pos = @schema.position(i.name)
      if pos.nil? or pos < 0 then
        append(i, value)
      else
        set_value(pos, value)
      end
    end
  end
  
  def type(name)
    @schema.type(name)
  end
  
  def timestamp=(value)
    @timestamp = value
  end
  
  def timestamp
    @timestamp
  end
  
  def join (inner)
    jointup = Tuple.new
    
    # take care of all join variables first
    @schema.variables.each do |v|
      # if (variable instanceof DontCare) {
      #   continue;
      # }
		  if (inner.schema.contains(v)) then
		    outerval = value(v.name)
		    innerval = inner.value(v.name)
		    if (outerval.nil? or innerval.nil?) then
		      if (outerval == innerval) then
		        jointup.append(v, nil)
	        else
	          return nil # tuples do not join
	        end
        elsif not (value(v.name)==inner.value(v.name))
          return nil # tuples do not join
        else
          jointup.append(v, value(v.name))
        end
	    else
	      # inner does not contain variable, so just add it
	      jointup.append(v, value(v.name))
      end
    end

    # Append any variables from the inner that do 
		# not match join variable.
		inner.schema.variables.each do |v|
      # if (variable instanceof DontCare) {
      #   continue;
      # }
			if (!jointup.schema.contains(v))
			  jointup.append(v, inner.value(v.name))
		  end
	  end
	  return jointup
  end
end