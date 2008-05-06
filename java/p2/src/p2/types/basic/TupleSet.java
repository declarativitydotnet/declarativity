package p2.types.basic;

import java.util.HashSet;
import java.util.Set;


public class TupleSet extends HashSet<Tuple> implements Comparable<TupleSet> {
	
	private String name;
	
	public TupleSet(String name) {
		this.name = name;
	}
	
	public TupleSet(String name, Set<Tuple> tuples) {
		this.name = name;
		this.addAll(tuples);
	}
	
	public String name() {
		return this.name;
	}

	/**
	 * The only meaningful response to this
	 * method is to determine if the two sets
	 * are equal.
	 */
	public int compareTo(TupleSet tuples) {
		if (tuples.containsAll(this)) {
			return 0;
		}
		return name().compareTo(tuples.name());
	}
}
	
