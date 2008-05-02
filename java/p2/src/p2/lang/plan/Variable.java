package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;

public class Variable extends Expression {
	public final static String DONTCARE = "_";

	private String name;
	
	private Class type;
	
	public Variable(String name, Class type) {
		this.name = name;
		this.type = type;
	}
	
	public static Variable dontCare(Class type) {
		return new Variable(DONTCARE, type);
	}
	
	public boolean isDontCare() {
		return DONTCARE.equals(this.name);
	}
	
	@Override
	public int hashCode() {
		return name().hashCode();
	}
	
	@Override
	public String toString() {
		return name();
	}
	
	public String name() {
		return this.name;
	}

	@Override
	public Class type() {
		return this.type;
	}
	
	public void type(Class type) {
		this.type = type;
	}

	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		variables.add(this);
		return variables;
	}
	
}
