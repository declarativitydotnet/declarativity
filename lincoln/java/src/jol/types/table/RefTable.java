package jol.types.table;

import java.util.Hashtable;
import java.util.Iterator;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;

/**
 * A RefCount Table (RefTable).
 * Tables of this type maintain a reference count on
 * each stored tuple. Insertions into this table create
 * a new tuple with a refcount of 1 or increment the refcount
 * of a preexisting tuple. Deletions into this table decrement
 * the refcount of the deleted tuple and remove that tuple if
 * the refcount goes to 0.
 *
 */
public class RefTable extends Table {
	/** The set of tuples stored by this table instance. */
	protected TupleSet tuples;
	
	/** A reference to the primary index. */
	protected Index primary;
	
	/** A map to all secondary indices. */
	protected Hashtable<Key, Index> secondary;
	
	/**
	 * Create a new RefCounted Table.
	 * @param context The runtime context.
	 * @param name The name of the table.
	 * @param key The primary key.
	 * @param types The list of attribute types.
	 */
	public RefTable(Runtime context, TableName name, Key key, TypeList types) {
		super(name, Type.TABLE, key, types);
		this.tuples = new TupleSet(name);
		this.primary = new HashIndex(context, this, key, Index.Type.PRIMARY);
		this.secondary = new Hashtable<Key, Index>();
	}
	
	@Override
	public Iterator<Tuple> tuples() {
		return (this.tuples == null ? new TupleSet(name()) : this.tuples.clone()).iterator();
	}
	
	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		if (this.tuples.contains(t)) {
			TupleSet lookup;
			try {
				lookup = primary().lookupByKey(primary().key().project(t));
			} catch (BadKeyException e) {
				throw new UpdateException(e.toString());
			}
		
			for (Tuple l : lookup) {
				if (l.equals(t)) {
					if (l.refCount() > 1L) {
						l.refCount(l.refCount() - 1L);
						return false; // The tuple has not been deleted yet.
					}
					return this.tuples.remove(t);
				}
			}
		}
		return false; // The tuple does not exist.
	}


	@Override
	protected boolean insert(Tuple t) throws UpdateException {
		if (this.tuples.contains(t)) {
			try {
				for (Tuple lookup : primary().lookupByKey(primary().key().project(t))) {
					if (lookup.equals(t)) {
						lookup.refCount(lookup.refCount().longValue() + 1);
						return false; 
					}
				}
			} catch (BadKeyException e) {
				throw new UpdateException(e.toString());
			}
		}
		this.tuples.add(t.clone());
		return true;
	}

	@Override
	public Index primary() {
		return this.primary;
	}

	@Override
	public Hashtable<Key, Index> secondary() {
		return this.secondary;
	}

	@Override
	public Integer cardinality() {
		return this.tuples.size();
	}

}
