package jol.types.basic;

import java.util.ArrayList;
import java.util.List;

import jol.lang.plan.DontCare;
import jol.lang.plan.Variable;
import jol.types.table.TableName;

/**
 * A schema a simply an ordered list of variables that
 * provide a name to tuple attributes. The variables
 * are taken from predicates in programs specific to
 * a rule.
 */
public class Schema {
	/** The table name that this schema references. */
	protected TableName name;

	/** Map from variable position to the variable itself */
	protected List<Variable> variables;

	/**  Create an empty schema. */
	public Schema() {
		this.name = null;
		this.variables = new ArrayList<Variable>();
	}

	/**
	 * Create a new schema.
	 * @param name The table name.
	 */
	public Schema(TableName name) {
		this.name = name;
		this.variables = new ArrayList<Variable>();
	}

	/**
	 * Create a new schema.
	 * @param name The table name
	 * @param variables A list of variables that make up this schema.
	 */
	public Schema(TableName name, List<Variable> variables) {
		this.name = name;
		for (int i = 0; i < variables.size(); i++) {
			variables.get(i).position(i);
		}
		this.variables = variables;
	}

	/**
	 * Copy constructor.
	 * @param schema The schema to copy.
	 */
	private Schema(Schema schema) {
		this.name = schema.name;
		this.variables = new ArrayList<Variable>(schema.variables);
	}

	@Override
	public Schema clone() {
		return new Schema(this);
	}

	/**
	 * The table name to which this schema refers.
	 * @return The table name.
	 */
	public TableName name() {
		return this.name;
	}

	/**
	 * The number of variables.
	 * @return A variable count.
	 */
	public int size() {
		return this.variables.size();
	}

	/**
	 * Append a variable to this schema.
	 * @param variable The variable to append.
	 */
	public void append(Variable variable) {
		variable.position(this.variables.size());
		this.variables.add(variable);
	}

	public boolean remove(Variable variable) {
		return this.variables.remove(variable);
	}

	/**
	 * Variable containment in this schema.
	 * @param variable The variable to test for containment.
	 * @return true if schema contains variable, false otherwise.
	 */
	public boolean contains(Variable variable) {
		return this.variables.contains(variable);
	}

	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		for (Variable var : variables()) {
			if (var instanceof DontCare) {
				sb.append("_");
			}
			else {
				sb.append(var.name());
			}
			sb.append(", ");
		}
		if (sb.lastIndexOf(", ") > 0) {
			return "(" + sb.substring(0, sb.lastIndexOf(", ")) + ")";
		}
		return "()";
	}


	/**
	 * An ordered list of the value types in this schema.
	 * @return A list of the value types.
	 */
	public final List<Class> types() {
		List<Class> types = new ArrayList<Class>();
		for (Variable variable : this.variables) {
			types.add(variable.type());
		}
		return types;
	}

	/**
	 * An ordered list of the variables that make up this schema.
	 * @return A list of the schema variables.
	 */
	public final List<Variable> variables() {
		return new ArrayList<Variable>(this.variables);
	}

	/**
	 * Find variable by name.
	 * @param name Variable name.
	 * @return The variable if exists, otherwise null.
	 * NOTE: use with caution. if more than 1 variable
	 * exists with the same name this method simply returns
	 * the first.
	 */
	public final Variable variable(String name) {
		for (Variable v : this.variables) {
			if (v.name().equals(name)) {
				return v;
			}
		}
		return null;
	}

	/**
	 * Get the value type by variable name.
	 * @param name The variable name.
	 * @return The value type.
	 */
	public final Class type(String name) {
		return variable(name).type();
	}

	/**
	 * Get the variable position by variable name.
	 * @param name The variable name.
	 * @return The position of the variable within this schema or -1 if !exist.
	 */
	public final int position(String name) {
		Variable v = variable(name);
		return v != null ? v.position() : -1;
	}

	/**
	 * Creates a new schema object based on a relation join
	 * operation with this schema.
	 * @param inner The schema to join with.
	 * @return The join schema.
	 */
	public final Schema join(Schema inner) {
		Schema join = new Schema();
		for (Variable variable : this.variables) {
			join.append((Variable)variable.clone());
		}

		for (Variable variable : inner.variables) {
			if (!join.contains(variable)) {
				join.append((Variable)variable.clone());
			}
		}
		return join;
	}
}
