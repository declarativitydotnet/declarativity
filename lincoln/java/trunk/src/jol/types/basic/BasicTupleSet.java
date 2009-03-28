package jol.types.basic;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;


/**
 * A tuple set is a set contain for tuples that belong to the same relation.
 */
public class BasicTupleSet implements TupleSet  {
	private static final long serialVersionUID = 1L;

	private Map<Tuple, Long> tuples;

	/** Identifier generator. */
	private static long idGen = 0L;

	/** Tuple set identifier. */
	private transient long id;

	private transient boolean warnedAboutBigTable = false;

	private transient boolean refCount = true;

	/**
	 * Create an empty tuple set.
	 */
	public BasicTupleSet() {
		this.id = idGen++;
		this.tuples = new HashMap<Tuple, Long>();
	}

	/**
	 * Copy constructor.
	 * @param clone The set to copy.
	 */
	private BasicTupleSet(TupleSet clone) {
		this.id = clone.id();
		this.tuples = new HashMap<Tuple, Long>();
		this.addAll(clone);
	}

	/**
	 * Initialize the tuple set to contain the passed in tuples
	 * that reference the given table name.
	 * @param name The table name.
	 * @param tuples The tuples to initialize.
	 */
	public BasicTupleSet(Collection<Tuple> tuples) {
		this.id = idGen++;
		this.tuples = new HashMap<Tuple, Long>();
		this.addAll(tuples);
	}

	/**
	 * Create a tuple set containing a single tuple.
	 * @param tuple A single tuple that will make up this set.
	 */
	public BasicTupleSet(Tuple tuple) {
		this.id = idGen++;
		this.tuples = new HashMap<Tuple, Long>();
		this.add(tuple);
	}

	public void refCount(boolean count) {
		this.refCount = count;
	}

	@Override
	public String toString() {
		String tuples = "[";
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

	public boolean addAll(Iterable<? extends Tuple> tuples) {
		for (Tuple t : tuples)
			add(t);

		if (this.size() > 20000 && !this.warnedAboutBigTable) {
			this.warnedAboutBigTable = true;
			System.err.println("TUPLE SET " + id + " contains " + size() + " tuples");
		}

		return true;
	}

	public boolean add(Tuple tuple) {
		if (tuple == null) {
			return false;
		} else if (this.tuples.containsKey(tuple)) {
			this.tuples.put(tuple, this.tuples.get(tuple) + 1L);
			return false;
		} else {
			this.tuples.put(tuple, 1L);
			return true;
		}
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
	    //if(!this.refCount) {
			return this.tuples.keySet().iterator();
			/*		} else {
			return new Iterator<Tuple>() {
				long count = 0L;
				Tuple next;
				Iterator<Tuple> it = tuples.keySet().iterator();
				@Override
				public boolean hasNext() {
					return count != 0 || it.hasNext();
				}

				@Override
				public Tuple next() {
					if(count == 0) {
						next = it.next();
						count = tuples.get(next) - 1L;
						if(count < 0L) { throw new IllegalStateException(); }
					} else {
						count--;
					}
					return next;
				}

				@Override
				public void remove() {
					throw new UnsupportedOperationException("TupleSetIterators are immutable!");
					
				}
				
			};
			}*/
	}

	public boolean remove(Object o) {
		if (o != null && o instanceof Tuple) {
			Tuple other = (Tuple) o;
			if (this.tuples.containsKey(o)) {
				Long l = this.tuples.get(o);
				if((!this.refCount) || l == 1) {
					this.tuples.remove(o);
					return true;
				} else {
					l--;
					this.tuples.put(other, l);
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
		for (Tuple t : this.tuples.keySet()) {
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

	private void writeObject(ObjectOutputStream out) throws IOException {
		out.defaultWriteObject();
	}

	private void readObject(ObjectInputStream in) throws IOException {
		try {
			in.defaultReadObject();
		} catch (ClassNotFoundException e) {
			throw new IOException(e);
		}

		// We need to manually restore transient fields
		this.id = idGen++;
		this.warnedAboutBigTable = false;
		this.refCount = true;
	}
}

