package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import xtc.tree.Node;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.function.TupleFunction;

public class Variable extends Expression {

	protected String name;

	protected Class type;

	private boolean location;

	public Variable(Node node, String name, Class type) {
		super(node);
		this.name = name;
		this.type = type;
	}

	public Variable(Node node, String name, Class type, boolean location) {
		this(node, name, type);
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
		return new Variable(node(), name, type, location);
	}

	@Override
	public int hashCode() {
		return name().hashCode();
	}

	@Override
	public String toString() {
		String var = name();
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
	public TupleFunction function(Schema schema) throws PlannerException {
		final int position = schema.position(name());
		if (position < 0) {
			throw new PlannerException("Unknown variable name " + name());
		}
		
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				try {
					return tuple.value(position);
				} catch (Throwable t) {
					t.printStackTrace();
					System.err.println("UNKNOWN VARIABLE NAME " + name());
					System.err.println("ASSUMED POSITION " + position);
					System.err.println("TUPLE " + tuple);
					System.err.println("SIZE " + tuple.size());
					throw new JolRuntimeException (t.toString());
				}
			}

			public Class returnType() {
				return type;
			}
		};
	}
}
