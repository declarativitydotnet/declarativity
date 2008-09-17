require 'lib/types/basic/schema'
require 'lib/types/basic/tuple'
require 'lib/lang/plan/variable'
require 'lib/lang/plan/value'
require 'lib/types/table/table_name'
require 'lib/types/table/index'

require "rubygems"

class Tuple
  include Comparable
  
  @@idGen = 0
  def initialize(*values)
    init
    @values = Array.new
    values.each {|v| @values << v}
    @values
  end
  
  def clone
    t = super
    t.values = @values.clone
    @values.freeze
    return t
  end

  def init
    @schema = Schema.new(nil,nil)
    @count = 1
    @tid = @@idGen
    @@idGen += 1
  end
  
  attr_accessor :tid, :count, :schema, :values

  def append (var, val)
    var = var.clone
    var.position = schema.size
    @schema << var
    @values << val
  end

  def to_s
    out = "<"
    if (@values.size > 0) then
      @values.each_with_index do |value, i|
        out += ", " unless i == 0 
        #out += (value.nil? ? "nil".to_s : value.to_s) 
        if value.nil? 
          out += "nil"
        elsif (value.class <= Tuple) || (value.class <= UnsortedTupleSet) || (value.class <= Index) || (value.class <= Table)
          # stop; enough is enough
          out += "InternalObject:#{value.object_id.to_s}"
        else
          out += value.to_s
        end
      end
    else
      out += @tid.to_s
    end
    return out + ">"
  end

  def schema=(s)
    if s.size != size then
#      require 'ruby-debug'; debugger
      err = "Schema assignment does not match tuple arity!  schema " + s.to_s+" (vs. tuple values ["
      values.each {|v| err <<  v.to_s + ", " }
      err[err.length-1] = "]" 
      err << ": length "+size.to_s+")"
      raise err
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
  
  def hash
    return to_s.hash
  end
  
  def size() 
    return @values.length
  end
  
  def value(i)
    if i.class <= Numeric
      return values[i]
    else
      if @schema.position(i).nil?
        require 'ruby-debug'; debugger
        raise("field "+i.to_s+" does not exist in tuple") 
      end
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
  
  def tuple_type(name)
    @schema.schema_type(name)
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
