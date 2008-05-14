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
	protected boolean insert(Tuple t) throws UpdateException {
		return !this.tuples.contains(t);
	}
	
	@Override
	protected boolean remove(Tuple t) throws UpdateException {
		return this.tuples.contains(t);
	}

	@Override
	public boolean isEvent() {
		return false;
	}

}
