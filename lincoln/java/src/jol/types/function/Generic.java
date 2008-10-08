package jol.types.function;

import java.lang.reflect.Method;
import jol.types.basic.Tuple;
import jol.types.exception.P2RuntimeException;


public class Generic<Type> implements TupleFunction<Type> {
	
	/* The instance object or null if static method is given. */
	private TupleFunction<Object> obj;
	
	/* The generic function to be called. */
	private Method method;
	
	/* The arguments to the generic function. */
	private TupleFunction<Object>[] args;

	public Generic(TupleFunction<Object> obj, Method method, TupleFunction<Object>... args) {
		this.obj = obj;
		this.method = method;
		this.args = args;
	}
	
	public Type evaluate(Tuple tuple) throws P2RuntimeException {
		Object obj = this.obj != null ? this.obj.evaluate(tuple) : null;
		Object[] args = new Object[this.args.length];
		for (int i = 0; i < this.args.length; i++) {
			args[i] = this.args[i].evaluate(tuple);
		}
		try {
			return (Type) method.invoke(obj, args);
		} catch (Exception e) {
			// TODO Fatal error.
			e.printStackTrace();
		}
		return null;
	}

	public Class returnType() {
		return method.getReturnType();
	}

}
