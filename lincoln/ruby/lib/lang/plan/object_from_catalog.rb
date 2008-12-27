module ObjectFromCatalog

  def camelize(str)
    str = str.split('_')
    retval = str[0]
    retval[0] = retval.downcase[0]
    str[1..str.length].each { |s| retval += s.to_s.capitalize }
    return retval
  end
  
  def constants
    retval = Array.new
    classname =self.class.to_s + "::Field"
    consts = eval(classname).constants
    consts.each do |c|
      str = classname + "::" + c
      retval << [c, eval(str)]
    end
    return retval
  end

  def insert_tup(tuple)
    # needs to be done through eval since nested Field is class-dependent
	  object_position = eval self.class.to_s + "::Field::OBJECT"
    object = tuple.values[object_position]
    raise UpdateException, "Object nil in catalog tuple" if object.nil?
    constants.each do |c|
      method = camelize(c[0].downcase) + "="
      object.send method.to_sym, tuple.values[c[1]] unless c[0] == 'OBJECT'
    end
    return super(tuple)
  end

end