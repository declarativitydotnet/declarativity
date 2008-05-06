package p2.types.table;

import java.util.Hashtable;
import java.util.Iterator;
import java.util.Vector;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;


public class BasicTable extends Table {
	
	public BasicTable(String name, Integer size, Number lifetime, Key key, TypeList types) {
		super(name, size, lifetime.floatValue(), key, types);
	}
	
	@Override
	protected Tuple insert(Tuple t) throws UpdateException {
		for (Tuple lookup : primary().lookup(t)) {
			if (lookup.equals(t)) {
				// TODO update tuple timestamp.
				return null;
			}
		}
		
		Tuple previous = primary().insert(t);
		for (Index i : secondary().values()) {
			i.insert(t);
		}
		return previous;
	}
	
	@Override
	protected boolean remove(Tuple t) throws UpdateException {
		for (Tuple tmp : primary().lookup(t)) {
			if (tmp.equals(t)) {
				primary().remove(t);
				for (Index i : secondary().values()) {
					i.remove(t);
				}
				return true;
			}
		}
		return false;
	}

}
