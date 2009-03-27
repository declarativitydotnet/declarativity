package jol.types.table;

import java.util.Map;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;

/**
 * An event table.
 *
 * Event tables do not store tuples and simply return whatever is 
 * passed to them via in the insert routine as the delta set. It 
 * is an error to perform a deletion from an event table.
 */
public class EventTable extends Table {

	/**
	 * Create a new event table.
	 * @param name The event table name
	 * @param types The types assumed to make up the values of event tuple.
	 */
	public EventTable(TableName name, Class[] types) {
		super(name, Type.EVENT, null, types);
	}
	
	@Override
	/** Simply returns what is passed in as the delta set. */
	public TupleSet insert(TupleSet tuples, TupleSet deletions) throws UpdateException {
		for (Callback callback : this.callbacks) {
			callback.insertion(tuples);
		}
		return tuples;
	}
	
	@Override
	/** @exception UpdateException */
	public TupleSet delete(Iterable<Tuple> tuples) throws UpdateException {
		throw new UpdateException("Can't remove tuples from event table");
	}

	@Override
	/** @exception UpdateException */
	protected boolean insert(Tuple t) throws UpdateException {
		throw new UpdateException("Can't insert tuple in event table");
	}

	@Override
	/** @exception UpdateException */
	protected boolean delete(Tuple t) throws UpdateException {
		throw new UpdateException("Can't remove tuple from event table");
	}

	@Override
	/** @return null */
	public Index primary() {
		return null; // No tuple storage
	}

	@Override
	/** @return null */
	public Map<Key, Index> secondary() {
		return null; // No tuple storage
	}

	/** @return null */
	@Override
	public Iterable<Tuple> tuples() {
		return null;
	}

	@Override
	/** @return 0 */
	public Long cardinality() {
		return 0L;
	}
}
