package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;

public class Value<Type> extends Expression {

	private Type value;
	
	public Value(Type value) {
		this.value = value;
	}
	
	public Type value() {
		return this.value;
	}
	
	@Override
	public Class type() {
		return this.value.getClass();
	}
	
	@Override
	public String toString() {
		if (Integer.class.isAssignableFrom(type()) && 
			value().equals(Integer.MAX_VALUE)) {
			return "infinity";
		}
		return value().toString();
	}

	@Override
	public Set<Variable> variables() {
		return new HashSet<Variable>();
	}
}
