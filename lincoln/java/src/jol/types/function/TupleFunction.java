package jol.types.function;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;


public interface TupleFunction<Type> {

	/**
	 * Evaluate the function on the given tuple.
	 * @param tuple  The tuple argument.
	 * @return The object value of the function evaluation.
	 * @throws JolRuntimeException 
	 */
	public Type evaluate(Tuple tuple) throws JolRuntimeException;
	
	/**
	 * The return type
	 */
	public Class returnType();
	
}
