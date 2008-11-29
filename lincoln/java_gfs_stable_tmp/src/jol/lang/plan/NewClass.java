package jol.lang.plan;

import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

public class NewClass extends Expression {
	
	private Class type;
	
	private Constructor constructor;
	
	private List<Expression> arguments;
	
	public NewClass(Class type) {
		this.type = type;
	}

	@Override
	public Class type() {
		return this.type;
	}

	@Override
	public String toString() {
		String value = "new " + constructor.getName() + "(";
		if (arguments.size() == 0) {
			return value + ")";
		}
		value += arguments.get(0).toString();
		for (int i = 1; i < arguments.size(); i++) {
			value += ", " + arguments.get(i);
		}
		return value + ")";
	}
	
	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		for (Expression<?> arg : arguments) {
			variables.addAll(arg.variables());
		}
		return variables;
	}
	
	public void constructor(Constructor constructor) {
		this.constructor = constructor;
	}
	
	public void arguments(List<Expression> arguments) {
		this.arguments = arguments;
	}

	@Override
	public TupleFunction function() {
		final List<TupleFunction> argFunctions = new ArrayList<TupleFunction>();
		for (Expression argument : this.arguments) {
			argFunctions.add(argument.function());
		}
		
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				Object[] arguments = new Object[NewClass.this.arguments.size()];
				int index = 0;
				for (TupleFunction argFunction : argFunctions) {
					arguments[index++] = argFunction.evaluate(tuple);
				}
				try {
					return NewClass.this.constructor.newInstance(arguments);
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
