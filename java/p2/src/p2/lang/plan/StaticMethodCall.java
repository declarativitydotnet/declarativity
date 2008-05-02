package p2.lang.plan;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class StaticMethodCall extends Expression {
	
	private Class type;
	
	private Field field;
	
	private Method method;
	
	private List<Expression> arguments;

	public StaticMethodCall(Class type, Method method, List<Expression> arguments) {
		this.type = type;
		this.method = method;
		this.arguments = arguments;
	}
	
	public StaticMethodCall(Field field, Method method, List<Expression> arguments) {
		this.field = field;
		this.method = method;
		this.arguments = arguments;
	}
	
	@Override
	public String toString() {
		String name;
		if (field != null) {
			name = this.field.getName() + "." + method.getName();
		}
		else {
			name = type.getName() + "." + method.getName();
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
		for (Expression arg : arguments) {
			variables.addAll(arg.variables());
		}
		return variables;
	}

}
