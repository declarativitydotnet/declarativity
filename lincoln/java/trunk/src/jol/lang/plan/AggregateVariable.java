package jol.lang.plan;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

public class AggregateVariable extends Variable {
	public static final String STAR = "*";
	
	public AggregateVariable(String name, Class type) {
		super(name, type);
	}
	
	public AggregateVariable(Variable variable) {
		super(variable.name(), variable.type());
	}
	
	@Override
	public Expression clone() {
		return new AggregateVariable(name, type);
	}
	
	@Override
	public TupleFunction function() {
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				return name().equals(STAR) ? tuple.id() : tuple.value(name());
			}

			public Class returnType() {
				return type;
			}
		};
	}

}
