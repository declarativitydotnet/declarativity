package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;

import p2.types.function.TupleFunction;

/**
 * Generic aggregates are supported by calling a statful VOID method on
 * an object. That arguments indicate what is passed into the object.
 * @author tcondie
 *
 */
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
	
	public TupleFunction object() {
		return method.object().function();
	}

}
