package jol.lang.plan;

import xtc.tree.Node;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

public class AggregateVariable extends Variable {
	public static final String STAR = "*";
	
	public AggregateVariable(Node node, String name, Class type) {
		super(node, name, type);
	}
	
	public AggregateVariable(Variable variable) {
		super(variable.node(), variable.name(), variable.type());
	}
	
	@Override
	public Expression clone() {
		return new AggregateVariable(node(), name, type);
	}
	
	@Override
	public TupleFunction function(Schema schema) {
		if (name().equals(STAR)) {
			return new TupleFunction() {
				public Object evaluate(Tuple tuple) throws JolRuntimeException {
					return tuple;
				}
				public Class returnType() { return Long.class; }
			};
		}
		else {
			final int position = schema.position(name());
			return new TupleFunction() {
				public Object evaluate(Tuple tuple) throws JolRuntimeException {
					return tuple.value(position);
				}
				public Class returnType() { return type; }
			};
		}
	}

}
