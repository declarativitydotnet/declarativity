package jol.lang.plan;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

public class DontCare extends Variable {
	public final static String DONTCARE = "_";
	public static Long ids = 0L;

	public DontCare(Class type) {
		super("DC" + Long.toString(ids++), type);
	}
	
	@Override
	public TupleFunction function() {
		assert(position() >= 0);
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				return tuple.value(position());
			}

			public Class returnType() {
				return type;
			}
		};
	}

}
