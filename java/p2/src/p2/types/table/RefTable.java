package p2.types.table;

import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Vector;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Table.Callback;

public class RefTable extends Table {
	/* The primary key. */
	protected Key key;
	
	protected TupleSet tuples;
	
	protected Index primary;
	
	protected Hashtable<Key, Index> secondary;
	
	public RefTable(String name, Key key, TypeList types) {
		super(name, Type.TABLE, Catalog.INFINITY, Catalog.INFINITY.floatValue(), key, types);
		this.key = key;
		this.tuples = new TupleSet(name);
		this.primary = new HashIndex(this, key, Index.Type.PRIMARY);
		this.secondary = new Hashtable<Key, Index>();
	}
	
	public TupleSet tuples() {
		return this.tuples == null ? new TupleSet(name()) : this.tuples.clone();
	}
	
	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		if (this.tuples.contains(t)) {
			TupleSet lookup = primary().lookup(t);
			assert(lookup.size() <= 1);
		
			for (Tuple l : lookup) {
				if (l.equals(t) && l.frozen()) {
					if (l.refCount() > 1L) {
						l.refCount(l.refCount() - 1L);
						return false; // The tuple has not been deleted yet.
					}
					primary().remove(t);
					for (Index i : secondary().values()) {
						i.remove(t);
					}
					return true; // The tuple was deleted.
				}
			}
		}
		return false; // The tuple does not exist.
	}


	@Override
	protected boolean insert(Tuple t) throws UpdateException {
		if (this.tuples.contains(t)) {
			for (Tuple lookup : primary().lookup(t)) {
				if (lookup.equals(t)) {
					lookup.refCount(lookup.refCount().longValue() + 1);
					return false; 
				}
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
