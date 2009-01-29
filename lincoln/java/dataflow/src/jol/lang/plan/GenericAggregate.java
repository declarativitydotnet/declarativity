package jol.lang.plan;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import xtc.tree.Node;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;
import jol.types.basic.Schema;

/**
 * Generic aggregates are supported by calling a stateful VOID method on
 * an object. That arguments indicate what is passed into the object.
 */
public class GenericAggregate extends Aggregate {

	public GenericAggregate(Node node, MethodCall method) {
		super(node, method, "generic", method.object().type());
	}
	
	@Override
	public GenericAggregate clone() {
		return new GenericAggregate(node(), this.method);
	}
	
	@Override
	public String toString() {
		return "generic<" + method.method().getName() + ">";
	}
	
	@Override
	public TupleFunction function(Schema schema) throws PlannerException {
		return this.method.object().function(schema);
	}
	
	public TupleFunction function(final Object instance, Schema schema) throws PlannerException {
		final List<TupleFunction> argFunctions = new ArrayList<TupleFunction>();
		final Method method = this.method.method();
		for (Expression argument : this.method.arguments()) {
			argFunctions.add(argument.function(schema));
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
