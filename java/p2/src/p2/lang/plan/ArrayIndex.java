package p2.lang.plan;

import java.lang.reflect.Array;
import java.util.Set;

import p2.types.basic.Tuple;
import p2.types.exception.RuntimeException;
import p2.types.function.TupleFunction;

public class ArrayIndex extends Expression {
	
	private Expression array;
	
	private Integer index;
	
	public ArrayIndex(Expression array, Integer index) {
		assert(array.type().isArray());
		this.array = array;
		this.index = index;
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
	public TupleFunction function() {
		return new TupleFunction() {
			private final TupleFunction function = array.function();
			public Object evaluate(Tuple tuple) throws ArrayIndexOutOfBoundsException, IllegalArgumentException, RuntimeException {
				return Array.get(function.evaluate(tuple), index);
			}

			public Class returnType() {
				return type();
			}
			
		};
	}
}
