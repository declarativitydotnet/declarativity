package p2.lang.plan;

import p2.types.basic.Tuple;
import p2.types.function.TupleFunction;


public class Aggregate extends Variable {
	
	private String function;
	
	public Aggregate(String name, String function, Class type) {
		super(name, type);
		this.function = function;
	}
	
	public Aggregate clone() {
		return new Aggregate(name(), function, type());
	}

	public String toString() {
		return this.function + "<" + super.toString() + ">";
	}
	
	public String functionName() {
		return this.function;
	}
}
