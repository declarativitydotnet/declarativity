package p2.types.function;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Tuple;
import p2.types.exception.RuntimeException;
import p2.types.exception.TypeException;


public class Generic<Type> implements TupleFunction<Type> {
	
	/* The instance object or null if static method is given. */
	private TupleFunction<Object> obj;
	
	/* The generic function to be called. */
	private Method method;
	
	/* The arguments to the generic function. */
	private TupleFunction<Object>[] args;

	public Generic(TupleFunction<Object> obj, Method method, TupleFunction<Object>... args) throws TypeException {
		this.obj = obj;
		this.method = method;
		this.args = args;
	}
	
	public Type evaluate(Tuple tuple) throws RuntimeException {
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
