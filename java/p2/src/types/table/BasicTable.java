package types.table;

import java.util.Hashtable;
import java.util.Iterator;
import java.util.Vector;

import types.basic.Tuple;
import types.basic.TupleSet;
import types.exception.UpdateException;

public class BasicTable extends Table {
	
	private Index primary;
	
	private Hashtable<Key, Index> secondary;

	public BasicTable(Table.Name name, Schema schema, Integer size, Number lifetime, Key key) {
		super(name, schema, size, lifetime, key);
	}
	
	public Iterator<Tuple> iterator() {
		return primary.tuples();
	}
	
	@Override
	public Index primary() {
		return primary;
	}

	@Override
	public Hashtable<Key, Index> secondary() {
		return secondary;
	}
	
	@Override
	protected Tuple insert(Tuple t) throws UpdateException {
		for (Tuple lookup : primary.lookup(t)) {
			if (lookup.equals(t)) {
				// TODO update tuple timestamp.
				return null;
			}
		}
		
		Tuple previous = primary.insert(t);
		for (Index i : secondary.values()) {
			i.insert(t);
		}
		return previous;
	}
	
	@Override
	protected boolean remove(Tuple t) throws UpdateException {
		for (Tuple tmp : primary.lookup(t)) {
			if (tmp.equals(t)) {
				primary.remove(t);
				for (Index i : secondary.values()) {
					i.remove(t);
				}
				return true;
			}
		}
		return false;
	}

}
