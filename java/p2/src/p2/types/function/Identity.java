package p2.types.function;

import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Tuple;

public class Identity implements TupleFunction<Comparable> {

	private Comparable obj;
	
	public Identity(Comparable obj) {
		this.obj = obj;
	}
	
	public Comparable evaluate(Tuple tuple) {
		return this.obj;
	}

	public Set<Variable> requires() {
		// TODO Auto-generated method stub
		return null;
	}

}
