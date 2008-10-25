package jol.lang.plan;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;


public class Aggregate extends Variable {
	public static final String STAR = "*";
	
	private String function;
	
	private MethodCall method;
	
	public Aggregate(String name, String function, Class type) {
		super(name, type);
		this.function = function;
		this.method = null;
	}
	
	public Aggregate(MethodCall method, String function, Class type) {
		this((method == null ? null : method.method().getName()), function, type);
		this.method = method;
	}
	
	@Override
	public Aggregate clone() {
		return this.method == null ? new Aggregate(name(), function, type()) :
									 new Aggregate(method, function, type());
	}

	@Override
	public String toString() {
		return this.function + "<" + super.toString() + ">";
	}
	
	public String functionName() {
		return this.function;
	}
	
	@Override
	public TupleFunction function() {
		return this.method != null ?  method.function() :
			new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				return name().equals(STAR) ? tuple.id() : tuple.value(name());
			}

			public Class returnType() {
				return type;
			}
		};
	}
}
