package p2.types.basic;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import p2.lang.plan.Variable;


public class Tuple {
	private static Long idGen = new Long(0);
	
	protected String id;
	
	protected String name;
	
	protected List<Comparable> values;
	
	protected Long timestamp;
	
	protected Long refCount;
	
	protected boolean frozen;
	
	protected Schema schema;
	
	
	public Tuple(String name, Comparable... values) {
		this.name = name;
		this.schema = null;
		this.values = new ArrayList<Comparable>();
		this.refCount = new Long(1);
		this.frozen = false;
		this.id = idGen.toString();
		idGen += 1L;
		
		for (Comparable value : values) {
			this.values.add(value);
		}
	}
	
	public Tuple(String name, List<Comparable> values) {
		this.name = name;
		this.schema = null;
		this.values = values;
		this.refCount = new Long(1);
		this.frozen = false;
		this.id = idGen.toString();
		idGen += 1L;
	}
	
	public Tuple(String name) {
		this.name = name;
		this.schema = new Schema(name);
		this.values = new ArrayList<Comparable>();;
		this.refCount = new Long(1);
		this.frozen = false;
		this.id = idGen.toString();
		idGen += 1L;
	}
	
	@Override
	public Tuple clone() {
		Tuple copy = new Tuple(name());
		copy.values = this.values;
		copy.schema = this.schema;
		return copy;
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
		for (Comparable element : values) {
			value += ", " + (element == null ? "null" : element.toString());
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
	
	public void schema(Schema schema) {
		this.schema = schema;
	}
	
	public int compareTo(Object o) {
		if (o instanceof Tuple) {
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
		return hashCode() < o.hashCode() ? -1 : 1;
	}
	
	@Override
	public boolean equals(Object obj) {
		return obj instanceof Tuple && ((Tuple)obj).compareTo(this) == 0;
	}
	
	@Override
	public int hashCode() {
		return values.hashCode();
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
	
	public Tuple join(Tuple inner) {
		Tuple join = this.clone();
		
		for (Variable variable : join.schema().variables()) {
			if (inner.schema().contains(variable)) {
				if (!join.value(name).equals(inner.value(name))) {
					return null; // Tuples do not join
				}
			}
			else {
				join.append(inner.schema().variable(name), inner.value(name));
			}
		}
		
		return join;
	}
	
	public Tuple project(Schema schema) {
		Tuple projection = new Tuple(name());
		
		for (Variable variable : this.schema.variables()) {
			if (schema.contains(variable)) {
				projection.append(variable, value(variable.name()));
			}
			else {
				// TODO exception.
			}
		}
		return projection;
	}
}
