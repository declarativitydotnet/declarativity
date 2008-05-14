package p2.lang.plan;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import p2.types.basic.Tuple;
import p2.types.exception.RuntimeException;
import p2.types.function.TupleFunction;

public class ObjectReference extends Reference {
	
	private Expression object;
	
	private Field field;
	
	public ObjectReference(Expression object, Field field) {
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
			public Object evaluate(Tuple tuple) throws RuntimeException {
				try {
					Object instance = objectFunction.evaluate(tuple);
					return ObjectReference.this.field.get(instance);
				} catch (Exception e) {
					throw new RuntimeException(e.toString());
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
