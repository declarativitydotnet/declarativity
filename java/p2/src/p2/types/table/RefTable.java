package p2.types.table;

import java.util.Hashtable;
import java.util.Iterator;
import java.util.Vector;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;

public class RefTable extends Table {
	
	public RefTable(String name, Key key, TypeList types) {
		super(name, Catalog.INFINITY, Catalog.INFINITY.floatValue(), key, types);
	}
	
	@Override
	protected boolean remove(Tuple t) throws UpdateException {
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
		return false; // The tuple does not exist.
	}


	@Override
	protected Tuple insert(Tuple t) throws UpdateException {
		for (Tuple lookup : primary().lookup(t)) {
			if (lookup.equals(t)) {
				lookup.refCount(lookup.refCount().longValue() + 1);
				return lookup; 
			}
		}
		
		t.refCount(1L);
		
		Tuple previous = primary().insert(t);
		for (Index i : secondary().values()) {
			i.insert(t);
		}
		return previous;
	}
}
