package jol.types.table;

import java.util.Map;

import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;

/**
 * The table function interface.
 * 
 * Table functions accept {@link TupleSet} arguments and apply some
 * function over that set of tuples. A delta set of tuples are returned
 * as the function value. The name of the table function is accessible from
 * a program by applying the table function to a predicate argument. The table
 * function will be called with any delta tuples that result on the given
 * predicate argument. A rule in a program that contains a table function
 * will only trigger off of insertions/deletions from the predicate given
 * as argument to the table function.
 */
public abstract class Function extends Table {

	/**
	 * Create a new table function with the given name in the
	 * global namespace.
	 * @param name The name of the table function.
	 * @param types The list of types this table function assumes. Should
	 * be the same as the types given by the predicate over which this
	 * table function is to be applied.
	 */
	public Function(String name, Class[] types) {
		super(new TableName(GLOBALSCOPE, name), Type.FUNCTION, null, types);
	}
	
	@Override 
	public abstract TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException;

	@Override
	protected final boolean insert(Tuple t) throws UpdateException {
		throw new UpdateException("Can't remove tuples from a table function");
	}
	
	@Override
	protected final boolean delete(Tuple t) throws UpdateException {
		throw new UpdateException("Can't remove tuples from a table function");
	}
	
	@Override
	public Index primary() {
		return null;
	}

	@Override
	public Map<Key, Index> secondary() {
		return null;
	}

	@Override
	public Iterable<Tuple> tuples() {
		return null;
	}
	
	@Override
	public Long cardinality() {
		return 0L;
	}
}
