package jol.lang.plan;

import java.lang.reflect.Array;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.basic.Schema;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;

public class ArrayIndex extends Expression {
	
	private Expression<Variable> array;
	
	private Integer index;
	
	public ArrayIndex(Expression<Variable> array, Integer index) {
		assert(array.type().isArray());
		this.array = array;
		this.index = index;
	}
	
	@Override
	public Expression clone() {
		return new ArrayIndex(array, index);
	}
	
	@Override
	public Class type() {
		return array.type().getComponentType();
	}
	
	@Override
	public String toString() {
		return "(" + array.toString() + ")[" + index + "]";
	}

	@Override
	public Set<Variable> variables() {
		return array.variables();
	}

	@Override
	public TupleFunction function(Schema schema) throws PlannerException {
		final TupleFunction arrayFunc = array.function(schema);
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				return Array.get(arrayFunc.evaluate(tuple), index);
			}

			public Class returnType() { return type(); }
			
		};
	}
}
