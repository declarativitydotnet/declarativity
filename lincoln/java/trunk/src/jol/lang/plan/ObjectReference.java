package jol.lang.plan;

import java.lang.reflect.Field;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;
import jol.types.basic.Schema;

public class ObjectReference extends Reference {
	
	private Expression<Object> object;
	
	private Field field;
	
	public ObjectReference(Expression<Object> object, Field field) {
		super(field.isEnumConstant() ? Enum.class : field.getType(), 
			  object.toString() + "." + field.getName());
		this.object = object;
		this.field = field;
	}
	
	public Expression clone() {
		return new ObjectReference(object, field);
	}

	@Override
	public Set<Variable> variables() {
		return object.variables();
	}
	
	public Field field() {
		return this.field;
	}

	@Override
	public TupleFunction function(Schema schema) throws PlannerException {
		final TupleFunction objectFunction = this.object.function(schema);
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				try {
					Object instance = objectFunction.evaluate(tuple);
					return ObjectReference.this.field.get(instance);
				} catch (Exception e) {
					throw new JolRuntimeException(e.toString());
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
