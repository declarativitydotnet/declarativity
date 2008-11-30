package jol.types.basic;

import java.util.HashSet;

public class ComparableSet<E> extends HashSet<E> implements Comparable<ComparableSet<E>> {
    private static final long serialVersionUID = 1L;

    public int compareTo(ComparableSet<E> o) {
		if (this.size() == o.size() && this.containsAll(o))  {
			return 0;
		}
		return -1;
	}
}
