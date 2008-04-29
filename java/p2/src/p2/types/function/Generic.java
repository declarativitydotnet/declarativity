package p2.types.function;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import p2.types.basic.Tuple;
import p2.types.exception.TypeException;


public class Generic<Type> implements Function<Type> {
	
	/* The instance object or null if static method is given. */
	private Function<Object> obj;
	
	/* The generic function to be called. */
	private Method method;
	
	/* The arguments to the generic function. */
	private Function<Object>[] args;

	public Generic(Function<Object> obj, Method method, Function<Object>... args) throws TypeException {
		this.obj = obj;
		this.method = method;
		this.args = args;
	}
	
	public Type eval(Tuple tuple) {
		Object obj = this.obj != null ? this.obj.eval(tuple) : null;
		Object[] args = new Object[this.args.length];
		for (int i = 0; i < this.args.length; i++) {
			args[i] = this.args[i].eval(tuple);
		}
		try {
			return (Type) method.invoke(obj, args);
		} catch (Exception e) {
			// TODO Fatal error.
			e.printStackTrace();
		}
		return null;
	}

}
