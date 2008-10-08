package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;

import p2.types.function.TupleFunction;

public class UnknownReference extends Reference {

	private Expression object;
	
	public UnknownReference(Expression object, Class type, String name) {
		super(type, name);
		this.object = object;
	}
	@Override
	public Expression object() {
		return this.object;
	}

	@Override
	public TupleFunction function() {
		return null;
	}

	@Override
	public Set<Variable> variables() {
		return object != null ? object.variables() : new HashSet<Variable>();
	}
}
