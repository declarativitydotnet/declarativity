package p2.lang.plan;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class VariableReference extends Expression {
	
	private Variable variable;
	
	private Field field;
	
	private String method;
	
	public VariableReference(Variable variable, Field field, String method) {
		this.variable = variable;
		this.field = field;
		this.method = method;
	}

	@Override
	public String toString() {
		if (field != null) {
			return variable.toString() + "." + field.getName();
		}
		else {
			return variable.toString() + "." + method + "(...)";
		}
	}

	@Override
	public Class type() {
		return variable.type();
	}

	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		variables.add(variable);
		return variables;
	}
	
	public Variable variable() {
		return this.variable;
	}
	
	public Field field() {
		return this.field;
	}
	
	public String method() {
		return this.method;
	}

}
