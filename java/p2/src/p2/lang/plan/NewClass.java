package p2.lang.plan;

import java.lang.reflect.Constructor;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class NewClass extends Expression {
	
	private Constructor constructor;
	
	private List<Expression> arguments;
	
	public NewClass(Constructor constructor, List<Expression> arguments) {
		this.constructor = constructor;
		this.arguments = arguments;
	}

	@Override
	public Class type() {
		return constructor.getDeclaringClass();
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
		for (Expression arg : arguments) {
			variables.addAll(arg.variables());
		}
		return variables;
	}
}
