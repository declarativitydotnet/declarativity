package jol.types.basic;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import jol.lang.plan.DontCare;
import jol.lang.plan.Variable;
import jol.types.exception.P2RuntimeException;


public class Tuple implements Comparable<Tuple>, Serializable {
	private static final long serialVersionUID = 1L;

	private static Long idGen = new Long(0);
	
	protected String id;
	
	protected String code;
	
	protected List<Comparable> values;
	
	protected Long timestamp;
	
	protected Long refCount;
	
	protected Schema schema;
	
	
	public Tuple(Comparable... values) {
		initialize();
		this.values = new ArrayList<Comparable>();
		for (Comparable value : values) {
			this.values.add(value);
		}
	}
	
	public Tuple(List<Comparable> values) {
		initialize();
		this.values = new ArrayList<Comparable>(values);
	}
	
	public Tuple() {
		initialize();
		this.values = new ArrayList<Comparable>();
	}
	
	private void writeObject(ObjectOutputStream out) 
		throws IOException {
		out.writeObject(values);
	} 
	
	private void readObject(ObjectInputStream in) 
		throws IOException, ClassNotFoundException {
		List<Comparable> values = (List) in.readObject();
		this.values = values;
	}
	
	@Override
	public Tuple clone() {
		Tuple copy = new Tuple(this.values);
		copy.schema = this.schema != null ? this.schema.clone() : null;
		copy.id     = this.id;
		return copy;
	}
	
	private void initialize() {
		this.schema   = new Schema();
		this.refCount = new Long(1);
		this.id       = idGen.toString();
		idGen += 1L;
	}
	
	public String id() {
		return this.id;
	}
	
	public void id(String id) {
		this.id = id;
	}
	
	public void append(Variable variable, Comparable value) {
		variable = variable.clone();
		variable.position(this.schema.size());
		this.schema.append(variable);
		this.values.add(value);
	}
	
	public String toString() {
		String value = "<";
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
	
	public Schema schema() {
		return this.schema;
	}
	
	public void schema(Schema schema) throws P2RuntimeException {
		if (schema.size() != size()) {
			throw new P2RuntimeException("Schema " + schema.name() + schema + " does not match tuple arity! " +
					                     " Tuple: " + this + " size =? " + size());
		}
		this.schema = schema;
	}
	
	public int compareTo(Tuple other) {
		if (size() != other.size()) {
			return -1;
		}
		for (int i = 0; i < size(); i++) {
			if (values.get(i) == null || other.values.get(i) == null) {
				if (values.get(i) != other.values.get(i)) {
					return -1;
				}
			}
			else if (values.get(i).compareTo(other.values.get(i)) != 0) {
				return values.get(i).compareTo(other.values.get(i));
			}
		}
		return 0;
	}
	
	@Override
	public boolean equals(Object obj) {
		return obj instanceof Tuple && 
				((Tuple)obj).compareTo(this) == 0;
	}
	
	@Override
	public int hashCode() {
		if (this.values.size() == 0)
				return id.hashCode();

		StringBuilder sb = new StringBuilder();
		for (Comparable value : values) {
			if (value == null)
				sb.append("null".hashCode());
			else
				sb.append(value.hashCode());
		}
		return sb.toString().hashCode();
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
		int position = this.schema.position(variable.name());
		if (position < 0) {
			append(variable, value);
		}
		else {
			value(position, value);
		}
	}
	
	public Class type(String name) {
		return this.schema.type(name);
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
	
	public Tuple join(Tuple inner) {
		Tuple join = new Tuple();
		
		/* Take care of all join variables first. */
		for (Variable variable : schema().variables()) {
			if (variable instanceof DontCare) {
				continue;
			}
			else if (inner.schema().contains(variable)) {
				Comparable outerValue = value(variable.name());
				Comparable innerValue = inner.value(variable.name());
				if (outerValue == null || innerValue == null) {
					if (outerValue == innerValue) {
						join.append(variable, null);
					}
				}
				else if (!value(variable.name()).equals(inner.value(variable.name()))) {
					return null; // Tuples do not join
				}
				else {
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
}
