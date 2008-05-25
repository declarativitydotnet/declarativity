package p2.types.table;

import java.util.Hashtable;
import java.util.Iterator;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;

public class EventTable extends Table {

	public EventTable(String name, TypeList types) {
		super(name, Type.EVENT, 0, 0f, null, types);
	}
	
	public TupleSet insert(TupleSet tuples) throws UpdateException {
		return tuples;
	}
	
	public TupleSet delete(TupleSet tuples) throws UpdateException {
		return tuples;
	}

	protected boolean insert(Tuple t) throws UpdateException {
		throw new UpdateException("Can't insert tuple in event table");
	}

	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		throw new UpdateException("Can't remove tuple from event table");
	}

	@Override
	public Index primary() {
		return null;
	}

	@Override
	public Hashtable<Key, Index> secondary() {
		return null;
	}

	public TupleSet tuples() {
		return null;
	}

	@Override
	public Integer cardinality() {
		return 0;
	}

}
