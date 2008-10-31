package jol.lang.plan;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

/**
 * Generic aggregates are supported by calling a stateful VOID method on
 * an object. That arguments indicate what is passed into the object.
 * @author tcondie
 *
 */
public class GenericAggregate extends Aggregate {

	public GenericAggregate(MethodCall method) {
		super(method, "generic", method.object().type());
	}
	
	@Override
	public GenericAggregate clone() {
		return new GenericAggregate(this.method);
	}
	
	@Override
	public String toString() {
		return "generic<" + method.toString() + ">";
	}
	
	@Override
	public TupleFunction function() {
		return this.method.object().function();
	}
	
	public TupleFunction function(final Object instance) {
		final List<TupleFunction> argFunctions = new ArrayList<TupleFunction>();
		final Method method = this.method.method();
		for (Expression argument : this.method.arguments()) {
			argFunctions.add(argument.function());
		}
		
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				Object[] arguments = new Object[argFunctions.size()];
				int index = 0;
				for (TupleFunction argFunction : argFunctions) {
					arguments[index++] = argFunction.evaluate(tuple);
				}
				try {
					try {
						method.invoke(instance, arguments);
						return instance;
					} catch (InvocationTargetException e) {
						System.err.println(e.getTargetException().getMessage());
						e.getTargetException().printStackTrace();
						System.exit(0);
					} catch (Exception e) {
						System.err.println(e + ": method invocation " + method.toString());
						System.exit(0);
					}
					return null;
				} catch (Exception e) {
					e.printStackTrace();
					throw new JolRuntimeException(e.toString());
				}
			}

			public Class returnType() {
				return type();
			}
		};
	}
	
}
