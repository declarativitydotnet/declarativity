package jol.types.operator;

import jol.core.Runtime;
import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.table.Index;
import jol.types.table.Key;

/**
 * An index join uses an index to reduce the number of tuples
 * scanned along the inner table.
 */
public class IndexJoin extends Join {

	/** The lookup key used to obtain join matches from the inner relation. */
	private Key lookupKey;

	/** The index used to perform the inner relation lookup. */
	private Index index;

	/**
	 * Create a new index join operator.
	 * @param context The runtime context.
	 * @param predicate The (table) predicate referencing the inner relation.
	 * @param input The input tuple schema.
	 * @param lookupKey The lookup key.
	 * @param index The index.
	 * @throws PlannerException
	 */
	public IndexJoin(Runtime context, Predicate predicate, Schema input,
			         Key lookupKey, Index index) throws PlannerException {
		super(context, predicate, input);
		this.lookupKey = lookupKey;
		this.index = index;
	}

	@Override
	public String toString() {
		return "index join ";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		try {
			TupleSet result = new BasicTupleSet();
			for (Tuple outer : tuples) {
				TupleSet innerTuples = this.index.lookupByKey(lookupKey.project(outer));
				result.addAll(join(outer, innerTuples));
			}
			return result;
		} catch (Throwable t) {
			throw new JolRuntimeException("Index join failed! Index table " + index.table() + ".", t);
		}
	}
}
