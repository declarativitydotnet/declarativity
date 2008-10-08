package jol.lang.plan;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.exception.P2RuntimeException;
import jol.types.function.TupleFunction;

/**
 * Generic aggregates are supported by calling a statful VOID method on
 * an object. That arguments indicate what is passed into the object.
 * @author tcondie
 *
 */
public class GenericAggregate extends Aggregate {

	private MethodCall method;
	
	public GenericAggregate(MethodCall method) {
		super(STAR, method.method().getName(), method.object().type());
		this.method = method;
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
	public Set<Variable> variables() {
		return method.variables();
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
			public Object evaluate(Tuple tuple) throws P2RuntimeException {
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
					throw new P2RuntimeException(e.toString());
				}
			}

			public Class returnType() {
				return type();
			}
		};
	}
	
}
