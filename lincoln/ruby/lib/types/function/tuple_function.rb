class TupleFunction

	def evaluate(tuple)
	  throw "Abstract method TupleFunction.evaluate needs to be subclassed"
  end
	
  def returnType
    throw "Abstract method TupleFunction.returnType needs to be subclassed"
  end
end
