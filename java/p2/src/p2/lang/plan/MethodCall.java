package p2.lang.plan;

import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

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

	@Override
	public Class type() {
		return method.getReturnType();
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
		// TODO Auto-generated method stub
		return null;
	}

}
