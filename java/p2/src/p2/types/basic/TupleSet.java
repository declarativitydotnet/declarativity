package p2.types.basic;

import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;


public abstract class TupleSet implements Set<Tuple> {
	
	private Set<Tuple> tuples;

	private Integer[] sort;
	
	protected TupleSet(Set<Tuple> tuples, Integer[] sort) {
		this.tuples = tuples;
		this.sort = sort;
	}
	
	public Integer[] sort() {
		return this.sort;
	}
	
	public boolean add(Tuple t) {
		return tuples.add(t);
	}

	public boolean addAll(Collection<? extends Tuple> c) {
		return tuples.addAll(c);
	}

	public void clear() {
		tuples.clear();
	}

	public boolean contains(Object o) {
		return tuples.contains(o);
	}

	public boolean containsAll(Collection<?> c) {
		return tuples.containsAll(c);
	}

	public boolean isEmpty() {
		return tuples.isEmpty();
	}

	public Iterator<Tuple> iterator() {
		return tuples.iterator();
	}

	public boolean remove(Object o) {
		return tuples.remove(o);
	}

	public boolean removeAll(Collection<?> c) {
		return tuples.removeAll(c);
	}

	public boolean retainAll(Collection<?> c) {
		return tuples.retainAll(c);
	}

	public int size() {
		return tuples.size();
	}

	public Object[] toArray() {
		return tuples.toArray();
	}

	public <T> T[] toArray(T[] a) {
		return tuples.toArray(a);
	}
}
	
