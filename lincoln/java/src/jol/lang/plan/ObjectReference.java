package jol.lang.plan;

import java.lang.reflect.Field;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.exception.P2RuntimeException;
import jol.types.function.TupleFunction;

public class ObjectReference extends Reference {
	
	private Expression<Object> object;
	
	private Field field;
	
	public ObjectReference(Expression<Object> object, Field field) {
		super(field.isEnumConstant() ? Enum.class : field.getType(), 
			  object.toString() + "." + field.getName());
		this.object = object;
		this.field = field;
	}

	@Override
	public Set<Variable> variables() {
		return object.variables();
	}
	
	public Field field() {
		return this.field;
	}

	@Override
	public TupleFunction function() {
		final TupleFunction objectFunction = this.object.function();
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws P2RuntimeException {
				try {
					Object instance = objectFunction.evaluate(tuple);
					return ObjectReference.this.field.get(instance);
				} catch (Exception e) {
					throw new P2RuntimeException(e.toString());
				}
			}
			public Class returnType() {
				return ObjectReference.this.field.getType();
			}
		};
	}

	@Override
	public Expression object() {
		return this.object;
	}
}
