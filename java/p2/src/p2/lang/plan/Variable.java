package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;
import p2.types.exception.RuntimeException;
import p2.types.basic.Tuple;
import p2.types.function.TupleFunction;

public class Variable extends Expression {

	protected String name;
	
	protected Class type;
	
	public Variable(String name, Class type) {
		this.name = name;
		this.type = type;
		position(-1);
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj instanceof Variable) {
			Variable other = (Variable) obj;
			return name().equals(other.name());
		}
		return false;
	}
	
	@Override
	public Variable clone() {
		Variable variable = new Variable(name, type);
		return variable;
	}
	
	@Override
	public int hashCode() {
		return name().hashCode();
	}
	
	@Override
	public String toString() {
		return name();
	}
	
	public String name() {
		return this.name;
	}

	@Override
	public Class type() {
		return this.type;
	}
	
	public void type(Class type) {
		this.type = type;
	}
	
	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		variables.add(this);
		return variables;
	}

	@Override
	public TupleFunction function() {
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws RuntimeException {
				if (tuple == null || !tuple.schema().contains(Variable.this))
					throw new RuntimeException("Varaible " + this + " not in tuple " + tuple);
				return tuple.value(name());
			}

			public Class returnType() {
				return type;
			}
		};
	}
	
}
