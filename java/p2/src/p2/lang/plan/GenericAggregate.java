package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;

import p2.types.function.TupleFunction;

public class GenericAggregate extends Aggregate {

	private MethodCall method;
	
	public GenericAggregate(MethodCall method) {
		super(STAR, method.method().getName(), method.object().type());
		this.method = method;
	}
	
	@Override
	public GenericAggregate clone() {
		return new GenericAggregate(this.method);
	}

	@Override
	public String toString() {
		return "generic<" + method.toString() + ">";
	}
	
	@Override
	public Set<Variable> variables() {
		return method.variables();
	}
	
	@Override
	public TupleFunction function() {
		return method.function();
	}

}
