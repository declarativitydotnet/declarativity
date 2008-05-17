package p2.types.basic;

import java.util.ArrayList;
import java.util.List;

import p2.lang.plan.DontCare;
import p2.lang.plan.Variable;
import p2.types.exception.P2RuntimeException;


public class Tuple implements Comparable<Tuple> {
	private static Long idGen = new Long(0);
	
	protected String id;
	
	protected String code;
	
	protected String name;
	
	protected List<Comparable> values;
	
	protected Long timestamp;
	
	protected Long refCount;
	
	protected boolean frozen;
	
	protected Schema schema;
	
	
	public Tuple(String name, Comparable... values) {
		List<Comparable> valueList = new ArrayList<Comparable>();
		for (Comparable value : values) {
			valueList.add(value);
		}
		initialize(name, valueList);
	}
	
	public Tuple(String name, List<Comparable> values) {
		initialize(name, values);
	}
	
	public Tuple(String name) {
		initialize(name, new ArrayList<Comparable>());
		this.schema = new Schema(name);
	}
	
	@Override
	public Tuple clone() {
		Tuple copy = new Tuple(name(), this.values);
		copy.schema = this.schema;
		return copy;
	}
	
	private void initialize(String name, List<Comparable> values) {
		this.name = name;
		this.schema = null;
		this.values = values;
		this.refCount = new Long(1);
		this.frozen = false;
		this.id = idGen.toString();
		idGen += 1L;
	}
	
	public String id() {
		return this.id;
	}
	
	public void append(Variable variable, Comparable value) {
		this.values.add(value);
		this.schema.append(variable);
	}
	
	public String toString() {
		String value = name() + "<";
		if (values.size() > 0) {
			value += values.get(0);
			for (int i = 1; i < values.size(); i++) {
				Comparable element = values.get(i);
				value += ", " + (element == null ? "null" : element.toString());
			}
		}
		value += ">";
		return value;
	}
	
	public String name() {
		return this.name;
	}
	
	public Schema schema() {
		return this.schema;
	}
	
	public void schema(Schema schema) throws P2RuntimeException {
		if (schema.size() != size()) {
			throw new P2RuntimeException("Schema assignment does not match tuple arity! " +
					                   "tuple name = " +  name() + " schema " + schema);
		}
		else if (!schema.name().equals(name())) {
			throw new P2RuntimeException("Schema assignment does not match tuple name " + 
					                    name() + "! " + schema);
		}
		this.schema = schema;
	}
	
	public int compareTo(Tuple o) {
			Tuple other = (Tuple) o;
			if (values.size() < other.values.size()) {
				return -1;
			}
			else if (other.values.size() < values.size()) {
				return 1;
			}
			else {
				for (int i = 0; i < values.size(); i++) {
					int valueCompare = values.get(i).compareTo(other.values.get(i));
					if (valueCompare != 0) {
						return valueCompare;
					}
				}
				return 0;
			}
	}
	
	@Override
	public boolean equals(Object obj) {
		return obj instanceof Tuple && ((Tuple)obj).compareTo(this) == 0;
	}
	
	@Override
	public int hashCode() {
		String code = "";
		for (Comparable value : this.values) {
			code += Integer.toString(value.hashCode());
		}
		return code.hashCode();
	}
	
	/* The number of attributes in this tuple. */
	public int size() {
		return this.values.size();
	}
	
	/* The value at the indicated field position. */
	public Comparable value(int field) {
		return values.get(field);
	}
	
	/* Sets the value to be in the given field position. */
	public void value(int field, Comparable value) {
		assert(field < values.size());
		values.set(field, value);
	}
	
	public Comparable value(String name) {
		if (this.schema != null) {
			return value(this.schema.position(name));
		}
		return null;
	}
	
	public void value(Variable variable, Comparable value) {
		if (this.schema != null) {
			int position = this.schema.position(variable.name());
			if (position < 0) {
				append(variable, value);
			}
			else {
				value(position, value);
			}
		}
	}
	
	public Class type(String name) {
		if (this.schema != null) {
			return this.schema.type(name);
		}
		return null;
	}
	
	public void refCount(Long value) {
		this.refCount = value;
	}
	
	public Long refCount() {
		return this.refCount;
	}
	
	public void timestamp(Long value) {
		this.timestamp = value;
	}
	
	public Long timestamp() {
		return this.timestamp;
	}
	
	public void frozen(boolean value) {
		this.frozen = value;
	}
	
	public boolean frozen() {
		return this.frozen;
	}
	
	public Tuple join(String name, Tuple inner) throws P2RuntimeException {
		Tuple join = new Tuple(name);
		// System.err.println("\tPERFORM " + name + ": outer schema " + schema() + " inner schema " + inner.schema());
		
		/* Take care of all join variables first. */
		for (Variable variable : schema().variables()) {
			if (variable instanceof DontCare) {
				continue;
			}
			else if (inner.schema().contains(variable)) {
				// System.err.println("\t\tVARIABLE MATCH " + variable);
				if (!value(variable.name()).equals(inner.value(variable.name()))) {
					/*
					System.err.println("\t\t\tVALUE FAIL " + 
							variable + " outer = " + value(variable.name()) + 
							" inner = " + inner.value(variable.name()));
							*/
					return null; // Tuples do not join
				}
				else {
					/*
					System.err.println("\t\t\tVALUE MATCH " + 
							variable + " outer = " + value(variable.name()) + 
							" inner = " + inner.value(variable.name()));
							*/
					join.append(variable, value(variable.name()));
				}
			}
			else {
				/* Inner does not contain variable so just add it. */
				join.append(variable, value(variable.name()));
			}
		}
		
		/* Append any variables from the inner that do 
		 * not match join variable. */
		for (Variable variable : inner.schema().variables()) {
			if (variable instanceof DontCare) {
				continue;
			}
			else if (!join.schema().contains(variable)) {
				join.append(variable, inner.value(variable.name()));
			}
		}
		return join;
	}
	
	public Tuple project(Schema schema) throws P2RuntimeException {
		Tuple projection = new Tuple(schema.name());
		
		for (Variable variable : schema.variables()) {
			if (this.schema.contains(variable)) {
				projection.append(variable, value(variable.name()));
			}
			else {
				throw new P2RuntimeException("Unknown variable " + variable + 
						                     " in schema projection of tuple " + toString());
			}
		}
		return projection;
	}
}
