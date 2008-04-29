package p2.types.function;

import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Tuple;

/**
 *  Access method for tuple fields.
 */
public class TupleField implements TupleFunction<Comparable> {
	
	private int field;
	
	public TupleField(int field) {
		this.field = field;
	}

	public Comparable evaluate(Tuple tuple) {
		return tuple.value(this.field);
	}

	public Set<Variable> requires() {
		// TODO Auto-generated method stub
		return null;
	}

}
