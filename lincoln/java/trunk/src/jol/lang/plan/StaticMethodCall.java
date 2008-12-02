package jol.lang.plan;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

public class StaticMethodCall extends Expression {

	private Class type;

	private Field field;

	private Method method;

	private List<Expression> arguments;

	public StaticMethodCall(Class type, Method method, List<Expression> arguments) {
		this.type = type;
		this.field = null;
		this.method = method;
		this.arguments = arguments;
	}

	public StaticMethodCall(Field field, Method method, List<Expression> arguments) {
		this.type = null;
		this.field = field;
		this.method = method;
		this.arguments = arguments;
	}

	public Expression clone() {
		List<Expression> arguments = new ArrayList<Expression>();
		for (Expression arg : this.arguments) {
			arguments.add(arg.clone());
		}
		return this.type == null ? 
				new StaticMethodCall(type, method, arguments) :
			    new StaticMethodCall(field, method, arguments);
	}
	
	@Override
	public String toString() {
		String name = "";
		if (field != null && method != null) {
			name = this.field.getName() + "." + method.getName();
		}
		else if (type != null && method != null) {
			name = type.getName() + "." + method.getName();
		}
		else {
			return name;
		}
		
		if (arguments.size() == 0) {
			return name + "()";
		}
		name += "(" + arguments.get(0).toString();
		for (int i = 1; i < arguments.size(); i++) {
			name += ", " + arguments.get(i);
		}
		return name + ")";
	}

	@Override
	public Class type() {
		return method.getReturnType();
	}

	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		for (Expression<?> arg : arguments) {
			variables.addAll(arg.variables());
		}
		return variables;
	}

	@Override
	public TupleFunction function() {
		final List<TupleFunction> argFunctions = new ArrayList<TupleFunction>();
		for (Expression argument : this.arguments) {
			argFunctions.add(argument.function());
		}

		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				Object[] arguments = new Object[StaticMethodCall.this.arguments.size()];
				int index = 0;
				for (TupleFunction argFunction : argFunctions) {
					arguments[index++] = argFunction.evaluate(tuple);
				}
				try {
					return StaticMethodCall.this.method.invoke(null, arguments);
				} catch (Exception e) {
					String msg = "ERROR: " + e.toString() +
					               ". Occurred while evaluating static method call \"" +
					               StaticMethodCall.this.toString() +
					               "\", on arguments " + Arrays.toString(arguments);
					throw new JolRuntimeException(msg, e);
				}
			}

			public Class returnType() {
				return type();
			}
		};
	}
}
