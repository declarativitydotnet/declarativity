package jol.types.table;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.exception.BadKeyException;

/**
 *  A hash implementation of an index. The key values are
 *  hashed using the {@link Tuple#hashCode()} value of the
 *  projected tuple.
 *  NOTE: Hash collisions are not handled by this structure.
 *  A user of this class must apply a further selection, which
 *  the query processor already does via the join operation.
 */
public class HashIndex extends Index {

	/** A map containing the set of tuples with the same key. */
	private Map<Tuple, BasicTupleSet> map;

	/**
	 * Create a new hash index. All tuples currently in
	 * the table will be added to the index by this constructor.
	 * @param context The runtime context.
	 * @param table The table that is to be indexed.
	 * @param key The key used to index the table.
	 * @param type The index type.
	 */
	public HashIndex(Runtime context, Table table, Key key, Type type) {
		super(context, table, key, type, true);
		this.map = new HashMap<Tuple, BasicTupleSet>();

		for (Tuple t : table.tuples()) {
			insert(t);
		}
	}

	@Override
	public String toString() {
		String out = "Index " + table().name() + "\n";
		if (map != null) {
			out += map.toString() + "\n";
		}
		return out;
	}

	@Override
	protected void insert(Tuple t) {
		Tuple key = key().project(t);
		BasicTupleSet tuples = this.map.get(key);

		if (tuples != null) {
		    tuples.add(t);
		} else {
			tuples = new BasicTupleSet();
			tuples.refCount(false);
			tuples.add(t);
			this.map.put(key, tuples);
		}
	}

	@Override
	public BasicTupleSet lookupByKey(Tuple key) throws BadKeyException {
		if (key.size() != key().size() && key().size() > 0) {
			throw new BadKeyException("Key had wrong number of columns.  " +
					"Saw: " + key.size() + " expected: " + key().size() + " key: " + key().toString());
		}

		BasicTupleSet tuples = this.map.get(key);
		if (tuples != null)
		    return tuples;
		else
		    return new BasicTupleSet();
	}

	@Override
	protected void remove(Tuple t) {
		Tuple key = key().project(t);

		BasicTupleSet tuples = this.map.get(key);
		if (tuples != null) {
		    tuples.remove(t);

		    if (tuples.isEmpty())
		        this.map.remove(key);
		}
	}

	@Override
	public Iterator<Tuple> iterator() {
		return new Iterator<Tuple>() {
			Iterator<BasicTupleSet> setIt = map.values().iterator();
			Iterator<Tuple> tupIt;
			@Override
			public boolean hasNext() {
				return (tupIt!=null && tupIt.hasNext()) || setIt.hasNext();
			}

			@Override
			public Tuple next() {
				if(tupIt == null || ! tupIt.hasNext()) {
					// Note: assumes that setIt.next() isn't empty.
					tupIt = setIt.next().iterator();
				}
				return tupIt.next();
			}

			@Override
			public void remove() {
				throw new UnsupportedOperationException("Index iterators do not support remove()!");
			}
		};
	}
}
