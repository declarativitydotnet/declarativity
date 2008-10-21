package jol.types.basic;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;

import jol.lang.plan.DontCare;
import jol.lang.plan.Variable;
import jol.types.table.TableName;

/**
 * A schema a simply an ordered list of variables that 
 * provide a name to tuple attributes. The variables
 * are taken from predicates in programs specific to
 * a rule.
 *
 */
public class Schema {
	
	/** The table name that this schema references. */
	protected TableName name;
	
	/** A hash from the string variable name to the variable itself. */
	protected Hashtable<String, Variable> variables;

	/**  Create an empty schema. */
	public Schema() {
		this.name = null;
		this.variables = new Hashtable<String, Variable>();
	}

	/** 
	 * Create a new schema.
	 * @param name The table name.
	 */
	public Schema(TableName name) {
		this.name = name;
		this.variables = new Hashtable<String, Variable>();
	}
	
	/**
	 * Create a new schema.
	 * @param name the table name
	 * @param variables a list of variables that make up this schema.
	 */
	public Schema(TableName name, List<Variable> variables) {
		this.name = name;
		for (int i = 0; i < variables.size(); i++) {
			variables.get(i).position(i);
		}
		this.variables = new Hashtable<String, Variable>();
		for (Variable variable : variables) {
			this.variables.put(variable.name(), variable);
		}
	}
	
	/**
	 * Copy constructor.
	 * @param schema The schema to copy.
	 */
	private Schema(Schema schema) {
		this.name = schema.name;
		this.variables = new Hashtable<String, Variable>(schema.variables);
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
		this.variables.put(variable.name(), variable);
	}
	
	/**
	 * Variable containment in this schema.
	 * @param variable The variable to test for containment.
	 * @return true if schema contains variable, false otherwise.
	 */
	public boolean contains(Variable variable) {
		return this.variables.containsKey(variable.name());
	}
	
	@Override
	public String toString() {
		return variables().toString();
	}
	
	
	/**
	 * An ordered list of the value types in this schema.
	 * @return A list of the value types.
	 */
	public final List<Class> types() {
		List<Class> types = new ArrayList<Class>();
		for (int position = 0; position < this.variables.size(); position++) {
			for (Variable variable : this.variables.values()) {
				if (variable.position() == position) {
					types.add(variable.type());
					break;
				}
			}
		}
		return types;
	}
	
	/**
	 * An ordered list of the variables that make up this schema.
	 * @return A list of the schema variables.
	 */
	public final List<Variable> variables() {
		List<Variable> variables = new ArrayList<Variable>();
		for (int position = 0; position < this.variables.size(); position++) {
			for (Variable variable : this.variables.values()) {
				if (variable.position() == position) {
					variables.add(variable);
					break;
				}
			}
		}
		return variables;
	}
	
	/**
	 * Find variable by name.
	 * @param name Variable name.
	 * @return The variable if exists, otherwise null.
	 */
	public final Variable variable(String name) {
		return this.variables.get(name);
	}
	
	/**
	 * Get the value type by variable name.
	 * @param name The variable name.
	 * @return The value type.
	 */
	public final Class type(String name) {
		return this.variables.get(name).type();
	}
	
	/**
	 * Get the variable position by variable name.
	 * @param name The variable name.
	 * @return The position of the variable within this schema or -1 if !exist.
	 */
	public final int position(String name) {
		return this.variables.containsKey(name) ?
				this.variables.get(name).position() : -1;
	}
	
	/**
	 * Creates a new schema object based on a relation join
	 * operation with this schema.
	 * @param inner The schema to join with.
	 * @return The join schema.
	 */
	public final Schema join(Schema inner) {
		Schema join = new Schema();
		for (Variable variable : this.variables()) {
			if (!(variable instanceof DontCare)) {
				join.append(variable.clone());
			}
		}
		
		for (Variable variable : inner.variables()) {
			if (!(variable instanceof DontCare) && !join.contains(variable)) {
				join.append(variable.clone());
			}
		}
		return join;
	}

}
