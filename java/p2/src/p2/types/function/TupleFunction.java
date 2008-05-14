package p2.types.function;

import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Tuple;
import p2.types.exception.RuntimeException;


public interface TupleFunction<Type> {

	/**
	 * Evaluate the function on the given tuple.
	 * @param tuple  The tuple argument.
	 * @return The object value of the function evaluation.
	 * @throws RuntimeException 
	 */
	public Type evaluate(Tuple tuple) throws RuntimeException;
	
	/**
	 * The return type
	 */
	public Class returnType();
	
}
