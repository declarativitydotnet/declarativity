package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;

import p2.types.function.TupleFunction;

public class Reference extends Expression {
	
	protected Class type;
	
	protected Expression object;
	
	protected String name;

	public Reference(String name) {
		this.type = null;
		this.object = null;
		this.name = name;
	}
	
	public Reference(Class type, String name) {
		this.type = type;
		this.object = null;
		this.name = name;
	}
	
	public Reference(Expression object, String name) {
		this.type = null;
		this.object = object;
		this.name = name;
	}

	@Override
	public String toString() {
		return this.name;
	}

	@Override
	public Class type() {
		return this.type;
	}
	
	public Expression object() {
		return this.object;
	}

	public Set<Variable> variables() {
		return new HashSet<Variable>();
	}

	@Override
	public TupleFunction function() {
		return null;
	}

}
