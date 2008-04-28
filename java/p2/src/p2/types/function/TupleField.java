package p2.types.function;

import p2.types.basic.Function;
import p2.types.basic.Tuple;

/**
 *  Access method for tuple fields.
 */
public class TupleField implements Function<Comparable> {
	
	private int field;
	
	public TupleField(int field) {
		this.field = field;
	}

	public Comparable eval(Tuple tuple) {
		return tuple.value(this.field);
	}

}
