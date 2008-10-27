package jol.types.basic;

import java.util.ArrayList;

/**
 * Simple data structure that stores a list of values
 * and is able to compare itself against another such list.
 * 
 * NOTE: used to add column lists to tuples.
 */
public class ValueList<T> extends ArrayList<T> implements Comparable<ValueList> {
	private static final long serialVersionUID = 1L;

	public int compareTo(ValueList o) {
		if (o.equals(this)) {
			return 0;
		}
		else {
			return -1;
		}
	}
	
	/**
	 * Insert into the list.
	 * @param o Element to insert.
	 */
	public void insert(T o) {
		super.add(o);
	}
	
}
