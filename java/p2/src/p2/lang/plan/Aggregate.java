package p2.lang.plan;

import p2.types.basic.Tuple;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;


public class Aggregate extends Variable {
	public static final String STAR = "*";
	
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
	
	@Override
	public TupleFunction function() {
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws P2RuntimeException {
				return name().equals(STAR) ? tuple.id() : tuple.value(name());
			}

			public Class returnType() {
				return type;
			}
		};
	}
}
