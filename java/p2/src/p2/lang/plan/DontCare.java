package p2.lang.plan;

import p2.types.basic.Tuple;
import p2.types.function.TupleFunction;

public class DontCare extends Variable {
	public final static String DONTCARE = "_";

	public DontCare(Class type) {
		super(DONTCARE, type);
	}
	
	@Override
	public boolean equals(Object obj) {
		return false;
	}

	@Override
	public TupleFunction function() {
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) {
				return tuple.value(position());
			}

			public Class returnType() {
				return type;
			}
			
		};
	}

}
