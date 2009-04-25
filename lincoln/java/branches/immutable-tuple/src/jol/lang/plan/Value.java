package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import xtc.tree.Node;

import jol.types.basic.Tuple;
import jol.types.function.TupleFunction;
import jol.types.basic.Schema;

public class Value<Type> extends Expression {

	private Type value;
	
	public Value(Node node, Type value) {
		super(node);
		this.value = value;
	}
	
	public Expression clone() {
		return new Value(node(), value);
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
		if (type() != null && Integer.class.isAssignableFrom(type()) && 
			value().equals(Integer.MAX_VALUE)) {
			return "infinity";
		}
        if (type() != null && type() == String.class) {
            return "\"" + this.value.toString() + "\"";
        }
		return this.value == null ? "null" : this.value.toString();
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
