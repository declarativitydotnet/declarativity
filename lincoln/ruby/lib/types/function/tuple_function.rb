class TupleFunction

	def evaluate(tuple)
	  raise "Abstract method TupleFunction.evaluate needs to be subclassed"
  end
	
  def returnType
    raise "Abstract method TupleFunction.returnType needs to be subclassed"
  end
end
