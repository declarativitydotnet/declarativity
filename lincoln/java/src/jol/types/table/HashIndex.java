package jol.types.table;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
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
	private Map<Tuple, TupleSet> map;

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
		this.map = new HashMap<Tuple, TupleSet>();
		
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
		if (this.map.containsKey(key)) {
			this.map.get(key).add(t);
		}
		else {
			TupleSet tuples = new TupleSet(table().name());
			tuples.add(t);
			this.map.put(key, tuples);
		}
	}

	@Override
	public TupleSet lookupByKey(Tuple key) throws BadKeyException {
		if (key.size() != key().size() && key().size() > 0) {
			throw new BadKeyException("Key had wrong number of columns.  " +
					"Saw: " + key.size() + " expected: " + key().size() + " key: " + key().toString());
		}
		return this.map.containsKey(key) ? 
				this.map.get(key) : new TupleSet(table().name());
	}

	@Override
	protected void remove(Tuple t) {
		Tuple key = key().project(t);
		
		if (this.map.containsKey(key)) {
			this.map.get(key).remove(t);
		}
	}

	@Override
	public Iterator<Tuple> iterator() {
		Set<Tuple> tuples = new HashSet<Tuple>();
		for (TupleSet set : this.map.values()) {
			tuples.addAll(set);
		}
		return tuples.iterator();
	}
}
