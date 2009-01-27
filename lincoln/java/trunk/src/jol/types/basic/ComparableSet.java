package jol.types.basic;

import java.util.HashSet;
import java.util.TreeSet;

public class ComparableSet<E> extends TreeSet<E> implements Comparable<ComparableSet<E>> {
    private static final long serialVersionUID = 1L;

	public ComparableSet<E> clone() {
		ComparableSet<E> clone = new ComparableSet<E>();
		clone.addAll(this);
		return clone;
	}
	
	public boolean equals (Object o) {
		if (o instanceof ComparableSet) {
			return compareTo((ComparableSet<E>)o) == 0;
		}
		return false;
	}
	
    public int compareTo(ComparableSet<E> o) {
		if (this.size() == o.size() && this.containsAll(o))  {
			return 0;
		}
		return -1;
	}
}
