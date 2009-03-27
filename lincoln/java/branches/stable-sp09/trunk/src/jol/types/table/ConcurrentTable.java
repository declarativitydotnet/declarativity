package jol.types.table;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import jol.core.Runtime;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.ConcurrentTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;

public class ConcurrentTable extends Table {
	/** The primary key. */
	protected Key key;

	/** The set of tuples stored by this table. */
	protected TupleSet tuples;

	/** The primary index. */
	protected Index primary;

	/** All secondary indices. */
	protected Map<Key, Index> secondary;

	/**
	 * Create a new basic table.
	 * @param context The runtime context.
	 * @param name The name of the table.
	 * @param key The primary key.
	 * @param types The type of each attribute (in positional order).
	 */
	public ConcurrentTable(Runtime context, TableName name, Key key, Class[] types) {
		super(name, Type.TABLE, key, types);
		this.key = key;
		this.tuples = new ConcurrentTupleSet();
		this.primary = new ConcurrentHashIndex(context, this, key, Index.Type.PRIMARY);
		this.secondary = new ConcurrentHashMap<Key, Index>();

		this.tuples.refCount(false);
	}

	@Override
	public Iterable<Tuple> tuples() {
		try {
			return (this.tuples == null ? new BasicTupleSet() : this.tuples.clone());
		} catch (Exception e) {
			System.err.println("TABLE " + name() + " ERROR: " + e);
			e.printStackTrace();
			System.exit(0);
		}
		return null;
	}

	@Override
	protected boolean insert(Tuple t) throws UpdateException {
		return this.tuples.add(t);
	}

	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		return this.tuples.remove(t);
	}

	@Override
	public Index primary() {
		return this.primary;
	}

	@Override
	public Map<Key, Index> secondary() {
		return this.secondary;
	}

	@Override
	public Long cardinality() {
		return this.tuples == null ? 0 : (long)this.tuples.size();
	}

}
