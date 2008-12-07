package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;
import jol.types.basic.Schema;

public class UnknownReference extends Reference {

	private Expression<?> object;
	
	public UnknownReference(Expression object, Class type, String name) {
		super(type, name);
		this.object = object;
	}
	
	public Expression clone() {
		return this;
	}
	
	@Override
	public Expression object() {
		return this.object;
	}

	@Override
	public TupleFunction function(Schema schema) throws PlannerException {
		throw new PlannerException("Unknown reference does not have a function!");
	}

	@Override
	public Set<Variable> variables() {
		return object != null ? object.variables() : new HashSet<Variable>();
	}
}
