package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.function.TupleFunction;
import jol.types.basic.Schema;

public class Value<Type> extends Expression {

	private Type value;
	
	public Value(Type value) {
		this.value = value;
	}
	
	public Expression clone() {
		return new Value(value);
	}
	
	public Type value() {
		return this.value;
	}
	
	@Override
	public Class type() {
		return this.value == null ? null : this.value.getClass();
	}
	
	@Override
	public String toString() {
		if (Integer.class.isAssignableFrom(type()) && 
			value().equals(Integer.MAX_VALUE)) {
			return "infinity";
		}
		return value().toString();
	}

	@Override
	public Set<Variable> variables() {
		return new HashSet<Variable>();
	}

	@Override
	public TupleFunction function(Schema schema) {
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) {
				return value;
			}
			public Class returnType() {
				return value.getClass();
			}
		};
	}
}
