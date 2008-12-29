class TupleFunction
  # /**
  #  * Evaluate the function on the given tuple.
  #  * @param tuple  The tuple argument.
  #  * @return The object value of the function evaluation.
  #  * @throws P2RuntimeException 
  #  */
	def evaluate(tuple)
	  raise "undefined abstract method TupleFunction.evaluate"
  end
	
  # /**
  #  * The return type
  #  */
	def returnType
	  raise "undefined abstract method TupleFunction.returnType"
  end
end