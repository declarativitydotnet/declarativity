package jol.types.table;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import jol.types.basic.Tuple;

/**
 * Identifies a set of attributes that make up some key
 * of a table.
 * An object of this type must be defined/created in order
 * to create an index (primary or secondary) of a table.
 */
public class Key implements Comparable<Key>, Iterable<Integer>, Serializable {
	private static final long serialVersionUID = 1L;

	/** List of integers that refer to the key attribute positions */
	private List<Integer> attributes;

	/**
	 * Create a new key with the given attribute positions.
	 * @param attr An array of the integer positions.
	 */
	public Key(Integer... attr) {
		this.attributes = new ArrayList<Integer>();
		for (Integer i : attr) {
			this.attributes.add(i);
		}
	}
	
	/**
	 * Create a new key with the given attribute positions.
	 * @param attr A list of the integer position.
	 */
	public Key(List<Integer> attr) {
		this.attributes = attr;
	}
	
	@Override
	public int hashCode() {
		return toString().hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof Key) {
			return toString().equals(((Key)o).toString());
		}
		return false;
	}
	
	@Override
	public String toString() {
		if (attributes.size() == 0) return "None";
		String value = this.attributes.get(0).toString();
		for (int i = 1; i < attributes.size(); i++) {
			value += ", " + attributes.get(i).toString();
		}
		return "Key("+value+")";
	}
	
	/**
	 * The number of attribute positions.
	 * @return The number of attribute positions.
	 */
	public int size() {
		return this.attributes.size();
	}
	
	/**
	 * Add a new integer position to the end of this key.
	 * @param position The integer position to be added.
	 */
	public void add(Integer position) {
		this.attributes.add(position);
	}
	
	/**
	 * Extract, from the tuple argument, the values
	 * at the integer positions represented by this key object.
	 * @param tuple The tuple containing the values to be extracted.
	 * @return A new tuple object containing only the extracted values 
	 * in key positional order.
	 */
	public Tuple project(Tuple tuple) {
		if (empty()) {
			return tuple;
		}
		else {
			List<Comparable> values = new ArrayList<Comparable>();
			for (Integer attr : attributes) {
				values.add(tuple.value(attr));
			}

			Tuple project = new Tuple(values);
			project.id(tuple.id());
			return project;
		}
	}
	
	public Tuple projectValue(Tuple tuple) {
		List<Comparable> values = new ArrayList<Comparable>();
		
		for(int i = 0; i < tuple.size(); i++) {
			if(!attributes.contains(i)) {
				values.add(tuple.value(i));
			}
		}

		Tuple project = new Tuple(values);
		project.id(tuple.id());
		return project;
	}
	
	public Tuple reconstruct(Tuple projectedKey, Tuple projectedValue) {
		int len = projectedKey.size() + projectedValue.size();
		List<Comparable> tuple= new ArrayList<Comparable>();
		int k = 0;
		
		assert(attributes.size() == projectedKey.size());
		
		for(Integer i: attributes) {
			while(tuple.size() <= i) { tuple.add(null); }
			tuple.set(i,projectedKey.value(k));
			k++;
		}
		int v = 0;
		for(int i = 0; i < len; i++) {
			if(!attributes.contains(i)) {
				while(tuple.size() <= i) { tuple.add(null); }
				tuple.set(i,projectedValue.value(v));
				v++;
			}
		}
		Tuple t = new Tuple(tuple);
		t.id(projectedKey.id());
		assert(t.id().equals(projectedValue.id()));
		return t;
	}

	public boolean empty() {
		return attributes.size() == 0;
	}
	
	public int compareTo(Key o) {
		if (attributes.size() < o.attributes.size()) {
			return -1;
		}
		else if (attributes.size() > o.attributes.size()) {
			return 1;
		}
		else {
			for (int i = 0; i < attributes.size(); i++) {
				int value = this.attributes.get(i).compareTo(o.attributes.get(i));
				if (value != 0) return value;
			}
			return 0;
		}
	}

	public Iterator<Integer> iterator() {
		return attributes.iterator();
	}
}
