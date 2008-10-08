package jol.types.function;

import jol.types.basic.Tuple;
import jol.types.exception.P2RuntimeException;


public interface TupleFunction<Type> {

	/**
	 * Evaluate the function on the given tuple.
	 * @param tuple  The tuple argument.
	 * @return The object value of the function evaluation.
	 * @throws P2RuntimeException 
	 */
	public Type evaluate(Tuple tuple) throws P2RuntimeException;
	
	/**
	 * The return type
	 */
	public Class returnType();
	
}
