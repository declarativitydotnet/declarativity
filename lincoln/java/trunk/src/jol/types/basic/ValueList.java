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

	public int compareTo(ValueList<T> other) {
		for (int i = 0; i < this.size(); i++) {
			/* If this value list has more elements than the other it
			 * comes after the other. */
			if (i == other.size()) return 1;
			int comp = this.get(i).compareTo(other.get(i));
			if (comp != 0) return comp;
		}
		/* If sizes match up then lists are equal. Otherwise
		 * the other value list has more elements and therefore
		 * this list comes before other. */
		return this.size() == other.size() ? 0 : -1;
	}

	/**
	 * Insert into the list.
	 * @param o Element to insert.
	 */
	public void insert(T o) {
		super.add(o);
	}
}
