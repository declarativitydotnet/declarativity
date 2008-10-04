package p2.types.function;

import p2.types.basic.Tuple;
import p2.types.exception.P2RuntimeException;


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
