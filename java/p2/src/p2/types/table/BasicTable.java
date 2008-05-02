package p2.types.table;

import java.util.Hashtable;
import java.util.Iterator;
import java.util.Vector;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.UpdateException;


public class BasicTable extends Table {
	
	public BasicTable(Table.Name name, Schema schema, Integer size, Number lifetime, Key key) {
		super(name, schema, size, lifetime, key);
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
