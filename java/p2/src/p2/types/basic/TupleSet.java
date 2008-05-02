package p2.types.basic;

import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.table.Schema;


public abstract class TupleSet extends Intermediate implements Set<Tuple> {
	
	private String name;
	
	private Schema schema;
	
	private Set<Tuple> tuples;
	
	protected TupleSet(String name, Schema schema, Set<Tuple> tuples) {
		this.name = name;
		this.schema = schema;
		this.tuples = tuples;
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
	
