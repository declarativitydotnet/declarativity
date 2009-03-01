package jol.types.basic;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import jol.types.table.TableName;


/**
 * A tuple set is a set contain for tuples that belong to the same relation.
 */
public class BasicTupleSet implements TupleSet  {
	private static final long serialVersionUID = 1L;

	private Map<Tuple, Tuple> tuples;

	/** Identifier generator. */
	private static long idGen = 0L;

	/** Tuple set identifier. */
	private long id;

	/** Table name to which the tuples of this set belong. */
	private TableName name;

	private boolean warnedAboutBigTable = false;

	private boolean refCount = true;

	/**
	 * Create an empty tuple set.
	 */
	public BasicTupleSet() {
		this((TableName) null);
	}

	/**
	 * Copy constructor.
	 * @param clone The set to copy.
	 */
	private BasicTupleSet(TupleSet clone) {
		this.id = clone.id();
		this.name = clone.name();
		this.tuples = new HashMap<Tuple, Tuple>();
		this.addAll(clone);
	}

	/**
	 * Empty tuple set that references a given table name.
	 * @param name The table name to reference.
	 */
	public BasicTupleSet(TableName name) {
		this.id = idGen++;
		this.name = name;
		this.tuples = new HashMap<Tuple, Tuple>();
	}

	/**
	 * Initialize the tuple set to contain the passed in tuples
	 * that reference the given table name.
	 * @param name The table name.
	 * @param tuples The tuples to initialize.
	 */
	public BasicTupleSet(TableName name, Set<Tuple> tuples) {
		this.id = idGen++;
		this.name = name;
		this.tuples = new HashMap<Tuple, Tuple>();
		this.addAll(tuples);
	}

	/**
	 * Create a tuple set referencing the given table name containing
	 * the single tuple.
	 * @param name The table name.
	 * @param tuple A single tuple that will make up this set.
	 */
	public BasicTupleSet(TableName name, Tuple tuple) {
		this.id = idGen++;
		this.name = name;
		this.tuples = new HashMap<Tuple, Tuple>();
		this.add(tuple);
	}

	public void refCount(boolean count) {
		this.refCount = count;
	}

	@Override
	public String toString() {
		String tuples = name + "[";
		Iterator<Tuple> iter = this.iterator();
		while (iter.hasNext()) {
			tuples += iter.next();
			if (iter.hasNext())
				tuples += ", ";
		}
		tuples += "]";
		return tuples;
	}

	@Override
	public int hashCode() {
	    // Take the XOR of the two halves of the ID
	    return (int) (this.id ^ (this.id >>> 32));
	}

	@Override
	public boolean equals(Object o) {
		if (o instanceof BasicTupleSet) {
			return ((BasicTupleSet) o).id == this.id;
		}
		return false;
	}

	@Override
	public TupleSet clone() {
		return new BasicTupleSet(this);
	}

	/**
	 * The tuple set identifier.
	 * @return The identifier assigned to this tuple set.
	 */
	public long id() {
		return this.id;
	}

	/**
	 * The name of the table that the tuples belonging to this set are
	 * part of.
	 * @return The table name.
	 */
	public TableName name() {
		return this.name;
	}

	/**
	 * Set the table name.
	 * @param name the table name.
	 */
	public void name(TableName name) {
		this.name = name;
	}

	public boolean addAll(Iterable<? extends Tuple> tuples) {
		for (Tuple t : tuples)
			add(t);

		if (this.size() > 20000 && !this.warnedAboutBigTable) {
			this.warnedAboutBigTable = true;
			System.err.println("TUPLE SET " + name() + " contains " + size() + " tuples");
		}

		return true;
	}

	public boolean add(Tuple tuple) {
		if (tuple == null) return false;
		else if (tuple.refCount <= 0) return false;
		else if (this.tuples.containsKey(tuple)) {
			this.tuples.get(tuple).refCountInc(tuple.refCount());
			return false;
		}
		tuple = tuple.clone();
		this.tuples.put(tuple, tuple);
		return true;
	}

	/**
	 * Comparison for tuple identifiers.
	 *
	 * NOTE: This method does NOT perform set comparison
	 * (other methods will perform that action {@link #containsAll(Collection)}).
	 */
	public int compareTo(TupleSet other) {
	    if (this.id < other.id())
	        return -1;
	    if (this.id > other.id())
	        return 1;

	    return 0;
	}

	public boolean addAll(Collection<? extends Tuple> c) {
		for (Tuple t : c) {
			add(t);
		}
		return true;
	}

	public void clear() {
		this.tuples.clear();
	}

	public boolean contains(Object o) {
		return this.tuples.containsKey(o);
	}

	public boolean containsAll(Collection<?> c) {
		for (Object t : c) {
			if (!this.tuples.containsKey(t)) {
				return false;
			}
		}
		return true;
	}

	public boolean isEmpty() {
		return this.tuples.isEmpty();
	}

	public Iterator<Tuple> iterator() {
		return this.tuples.values().iterator();
	}

	public boolean remove(Object o) {
		if (o != null && o instanceof Tuple) {
			Tuple other = (Tuple) o;
			if (this.tuples.containsKey(o)) {
				Tuple t = this.tuples.get(o);
				t.refCount(t.refCount - other.refCount());
				if (!this.refCount || t.refCount <= 0L) {
					this.tuples.remove(o);
					return true;
				}
			}
		}
		return false;
	}

	public boolean removeAll(Collection<?> c) {
		for (Object o : c) {
			remove(o);
		}
		return true;
	}

	public boolean retainAll(Collection<?> c) {
		List<Tuple> removal = new ArrayList<Tuple>();
		for (Tuple t : this.tuples.values()) {
			if (!c.contains(t)) {
				removal.add(t);
			}
		}
		for (Tuple t : removal) {
			remove(t);
		}
		return true;
	}

	public int size() {
		return this.tuples.size();
	}

	public Object[] toArray() {
		return this.tuples.values().toArray();
	}

	public <T> T[] toArray(T[] a) {
		return this.tuples.values().toArray(a);
	}
}

