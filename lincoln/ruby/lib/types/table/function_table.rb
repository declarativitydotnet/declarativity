require "lib/types/table/table"

# /**
#  * The table function interface.
#  * 
#  * Table functions accept {@link TupleSet} arguments and apply some
#  * function over that set of tuples. A delta set of tuples are returned
#  * as the function value. The name of the table function is accessible from
#  * a program by applying the table function to a predicate argument. The table
#  * function will be called with any delta tuples that result on the given
#  * predicate argument. A rule in a program that contains a table function
#  * will only trigger off of insertions/deletions from the predicate given
#  * as argument to the table function.
#  */
class TableFunctionTable < Table 

  # /**
  #  * Create a new table function with the given name in the
  #  * global namespace.
  #  * @param name The name of the table function.
  #  * @param types The list of types this table function assumes. Should
  #  * be the same as the types given by the predicate over which this
  #  * table function is to be applied.
  #  */
	def initialize(name, types)
		super(TableName.new(GLOBALSCOPE, name), TableType::FUNCTION, nil, types)
	end
	
  def insert(tuples, conflicts) 
    raise "undefined abstract function Function.insert"
  end

	def insert_tup(t)
		raise("Can't insert tuples into a table function")
	end
	

  def delete(t)
		raise("Can't remove tuples from a table function")
	end
	

  def primary
		nil
	end

  def secondary
    nil
	end

	def tuples
    nil
	end
	
  def cardinality
    0
	end
end
