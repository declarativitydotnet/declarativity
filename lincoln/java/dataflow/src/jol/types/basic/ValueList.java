package jol.types.basic;

import java.util.ArrayList;

/**
 * Simple data structure that stores a list of values
 * and is able to compare itself against another such list.
 * 
 * NOTE: used to add column lists to tuples.
 */
public class ValueList<T extends Comparable> extends ArrayList<T> implements Comparable<ValueList<T>> {
	private static final long serialVersionUID = 1L;

	public ValueList() {
		super();
	}
	
	public ValueList(T[] values) {
		super();
		for (T value : values) {
			add(value);
		}
	}
	
	public int compareTo(ValueList<T> o) {
		if (size() < o.size()) return -1;
		if (size() > o.size()) return 1;
		for (int i = 0; i < size(); i++) {
			int comp = this.get(i).compareTo(o.get(i));
			if (comp != 0) return comp;
		}
		return 0;
	}
	
	/**
	 * Insert into the list.
	 * @param o Element to insert.
	 */
	public void insert(T o) {
		super.add(o);
	}
	
}
