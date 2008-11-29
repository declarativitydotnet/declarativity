package jol.lang.plan;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

public class MethodCall extends Expression {
	
	private Expression<?> object;
	
	private Method method;
	
	private List<Expression> arguments;
	
	public MethodCall(Expression object, Method method, List<Expression> arguments) {
		this.object = object;
		this.method = method;
		this.arguments = arguments;
	}

	public Expression object() {
		return this.object;
	}
	
	public Method method() {
		return this.method;
	}
	
	public List<Expression> arguments() {
		return this.arguments;
	}
	
	@Override
	public Class type() {
		return method.getReturnType() == Void.class ? this.object.type() : method.getReturnType();
	}
	
	@Override
	public String toString() {
		String value = method.getName() + "(";
		if (arguments.size() == 0) {
			return object.toString() + "." + value + ")";
		}
		value += arguments.get(0).toString();
		for (int i = 1; i < arguments.size(); i++) {
			value += ", " + arguments.get(i);
		}
		return object.toString() + "." + value + ")";
	}

	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		variables.addAll(object.variables());
		for (Expression<?> arg : arguments) {
			variables.addAll(arg.variables());
		}
		return variables;
	}

	@Override
	public TupleFunction function() {
		final TupleFunction objectFunction = this.object.function();
		final List<TupleFunction> argFunctions = new ArrayList<TupleFunction>();
		for (Expression argument : this.arguments) {
			argFunctions.add(argument.function());
		}
		
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				Object instance = objectFunction.evaluate(tuple);
				Object[] arguments = new Object[MethodCall.this.arguments.size()];
				int index = 0;
				for (TupleFunction argFunction : argFunctions) {
					arguments[index++] = argFunction.evaluate(tuple);
				}
				try {
					if (MethodCall.this.method.getReturnType() == void.class) {
						MethodCall.this.method.invoke(instance, arguments);
						return instance;
					}
					else {
						return MethodCall.this.method.invoke(instance, arguments);
					}
				} catch (Throwable e) {
					String error = "ERROR: method invocation on " + 
							instance + " method " +
							MethodCall.this.method.toString() + 
							" arguments " + arguments;
					throw new JolRuntimeException(error);
				}
			}

			public Class returnType() {
				return type();
			}
		};
	}

}
