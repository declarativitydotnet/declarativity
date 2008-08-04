package p2.lang.plan;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import p2.types.basic.Tuple;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;

public class MethodCall extends Expression {
	
	private Expression object;
	
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
		for (Expression arg : arguments) {
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
			public Object evaluate(Tuple tuple) throws P2RuntimeException {
				Object instance = objectFunction.evaluate(tuple);
				Object[] arguments = new Object[MethodCall.this.arguments.size()];
				int index = 0;
				for (TupleFunction argFunction : argFunctions) {
					arguments[index++] = argFunction.evaluate(tuple);
				}
				try {
					try {
						if (MethodCall.this.method.getReturnType() == void.class) {
							MethodCall.this.method.invoke(instance, arguments);
							return instance;
						}
						else {
							return MethodCall.this.method.invoke(instance, arguments);
						}
					} catch (InvocationTargetException e) {
						System.err.println(e.getTargetException().getMessage());
						e.getTargetException().printStackTrace();
						System.exit(0);
					} catch (Exception e) {
						System.err.println(e + ": method invocation " + MethodCall.this.method.toString());
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
