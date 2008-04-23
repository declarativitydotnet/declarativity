package types.basic;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Vector;

public class Tuple implements Comparable {
	
	public final static class Property {
		public final static String NAME = "Name";
		public final static String PROGRAM = "Program";
	}
	
	private final Hashtable<String, Comparable> properties;
	
	protected List<Comparable> values;
	
	protected Long timestamp;
	
	protected Long refCount;
	
	protected boolean frozen;
	
	public Tuple(String name) {
		this.properties = new Hashtable<String, Comparable>();
		this.values = new ArrayList<Comparable>();
		this.refCount = new Long(1);
		this.frozen = false;
		property(Property.NAME, name);
	}
	
	public Tuple(String name, Comparable... values) {
		this.properties = new Hashtable<String, Comparable>();
		this.values = new ArrayList<Comparable>();
		this.refCount = new Long(1);
		this.frozen = false;
		property(Property.NAME, name);
		
		for (Comparable value : values) {
			this.values.add(value);
		}
	}
	
	public Tuple(String name, List<Comparable> values) {
		this.properties = new Hashtable<String, Comparable>();
		this.values = values;
		this.refCount = new Long(1);
		this.frozen = false;
		property(Property.NAME, name);
	}
	
	public String name() {
		return (String) property(Property.NAME);
	}
	
	public void property(String key, Comparable value) {
		this.properties.put(key, value);
	}
	
	public Comparable property(String key) {
		return this.properties.get(key);
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
	
	public static List<Comparable> project(Tuple t, Integer[] fields) {
		List<Comparable> values = new ArrayList<Comparable>();
		for (Integer field : fields) {
			values.add(t.value(field));
		}
		return values;
	}
	
	public static Tuple join(Tuple outer, Tuple inner, Integer[] innerJoinFields) {
		String name = "(" + outer.name() + ") join " + inner.name();
		Tuple join = new Tuple(name);
		for (int i = 1; i < outer.size(); i++) {
			join.value(join.size(), outer.value(i));
		}
		for (int i = 1; i < inner.size(); i++) {
			for (Integer field : innerJoinFields) {
				if (field.intValue() == i) {
					continue;
				}
			}
			// Not part of the join key, append to join tuple.
			join.value(join.size(), inner.value(i));
		}
		return join;
	}

}
