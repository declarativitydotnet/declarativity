package jol.lang.plan;

import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;
import xtc.tree.Node;

public class NewClass extends Expression {
	private Class type;
	private Constructor constructor;
	private List<Expression> arguments;

	public NewClass(Node node, Class type) {
		super(node);
		this.type = type;
	}

	private NewClass(Node node, Class type, Constructor constructor, List<Expression> arguments) {
		this(node, type);
		this.constructor = constructor;
		this.arguments = arguments;
	}

	public Expression clone() {
		List<Expression> arguments = new ArrayList<Expression>();
		for (Expression arg : this.arguments) {
			arguments.add(arg.clone());
		}
		return new NewClass(node(), type, constructor, arguments);
	}

	@Override
	public Class type() {
		return this.type;
	}

	@Override
	public String toString() {
		if (constructor == null) {
			return "Constructor class " + this.type;
		}
		
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
	public TupleFunction function(Schema schema) throws PlannerException {
		final List<TupleFunction> argFunctions = new ArrayList<TupleFunction>();
		for (Expression argument : this.arguments) {
			argFunctions.add(argument.function(schema));
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
