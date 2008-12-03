package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;
import jol.types.exception.JolRuntimeException;
import jol.types.basic.Tuple;
import jol.types.function.TupleFunction;

public class Variable extends Expression {

	protected String name;

	protected Class type;

	private boolean location;

	public Variable(String name, Class type) {
		this.name = name;
		this.type = type;
		position(-1);
	}

	public Variable(String name, Class type, boolean location) {
		this(name, type);
		this.location = location;
	}

	/**
	 * Is this variable a location variable?
	 * @return true if so, false otherwise
	 */
	public boolean loc() {
		return this.location;
	}

	public void loc(boolean loc) {
		this.location = loc;
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
	public Expression clone() {
		Variable variable = new Variable(name, type);
		variable.position(this.position());
		variable.location = this.location;
		return variable;
	}

	@Override
	public int hashCode() {
		return name().hashCode();
	}

	@Override
	public String toString() {
		String var = (position() >= 0) ? name() + ":" + position() : name();
		return this.location ? "@" + var : var;
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
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				try {
					return tuple.value(name());
				} catch (Throwable t) {
					System.err.println("UNKNOWN VARIABLE NAME " + name() + " IN TUPLE SCHEMA " + tuple.schema());
					System.err.println("ASSUMED POSITION " + position());
					t.printStackTrace();
					throw new JolRuntimeException (t.toString());
				}
			}

			public Class returnType() {
				return type;
			}
		};
	}
}
