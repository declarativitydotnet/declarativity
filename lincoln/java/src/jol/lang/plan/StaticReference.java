package jol.lang.plan;

import java.lang.reflect.Field;
import java.util.HashSet;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.exception.P2RuntimeException;
import jol.types.function.TupleFunction;

public class StaticReference extends Reference {
	
	private Field field;
	
	public StaticReference(Class type, Field field) {
		super(field.isEnumConstant() ? Enum.class : field.getType(), 
			  type.getCanonicalName() + "." + field.getName());
		this.field = field;
	}
	
	public Field field() {
		return this.field;
	}

	@Override
	public TupleFunction function() {
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws P2RuntimeException {
				try {
					return StaticReference.this.field.get(null);
				} catch (Exception e) {
					throw new P2RuntimeException(e.toString());
				}
			}
			public Class returnType() {
				return StaticReference.this.field.getType();
			}
		};
	}

	@Override
	public Set<Variable> variables() {
		return new HashSet<Variable>();
	}

	@Override
	public Expression object() {
		return null;
	}
}
