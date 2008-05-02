package p2.lang.plan;

import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class MethodCall extends Expression {
	
	private Variable variable;
	
	private Method method;
	
	private List<Expression> arguments;
	
	public MethodCall(Variable variable, Method method, List<Expression> arguments) {
		this.variable = variable;
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
			return variable.toString() + "." + value + ")";
		}
		value += arguments.get(0).toString();
		for (int i = 1; i < arguments.size(); i++) {
			value += ", " + arguments.get(i);
		}
		return variable.toString() + "." + value + ")";
	}

	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		variables.addAll(variable.variables());
		for (Expression arg : arguments) {
			variables.addAll(arg.variables());
		}
		return variables;
	}

}
