package jol.types.table;

import java.util.Hashtable;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;

public class RefTable extends Table {
	/* The primary key. */
	protected Key key;
	
	protected TupleSet tuples;
	
	protected Index primary;
	
	protected Hashtable<Key, Index> secondary;
	
	public RefTable(Runtime context, TableName name, Key key, TypeList types) {
		super(name, Type.TABLE, key, types);
		this.key = key;
		this.tuples = new TupleSet(name);
		this.primary = new HashIndex(context, this, key, Index.Type.PRIMARY);
		this.secondary = new Hashtable<Key, Index>();
	}
	
	public TupleSet tuples() {
		return this.tuples == null ? new TupleSet(name()) : this.tuples.clone();
	}
	
	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		if (this.tuples.contains(t)) {
			TupleSet lookup;
			try {
				lookup = primary().lookup(t);
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
				for (Tuple lookup : primary().lookup(t)) {
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
