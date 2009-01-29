package jol.types.operator;

import jol.core.Runtime;
import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.table.Index;
import jol.types.table.Key;

public class AntiIndexJoin extends Join {

	/** The lookup key used to obtain join matches from the inner relation. */
	private Key lookupKey;

	/** The index used to perform the inner relation lookup. */
	private Index index;

	/**
	 * Create a new operator.
	 * @param context The runtime context.
	 * @param predicate The (notin) predicate.
	 * @param input The input schema.
	 * @throws PlannerException
	 */
	public AntiIndexJoin(Runtime context,
			                 Predicate predicate,
			                 Schema input,
			                 Key lookupKey,
			                 Index index)
	throws PlannerException {
		super(context, predicate, input);
		this.lookupKey = lookupKey;
		this.index = index;
	}

	@Override
	public String toString() {
		return "anti nested-loop join";
	}

	@Override
	public TupleSet evaluate(TupleSet outerTuples) throws JolRuntimeException {
		try {
			TupleSet result = new TupleSet();
			for (Tuple outer : outerTuples) {
				TupleSet innerTuples = this.index.lookupByKey(lookupKey.project(outer));
				if (innerTuples.size() == 0) {
					/* An obvious optimized case. */
					result.add(outer);
				}
				else {
					/* We need to perform the actual join on the *reduced* tuple set. */
					outerTuples.add(outer);
					TupleSet join = join(outerTuples, innerTuples);
					if (join.size() == 0) result.add(outer);
				}
			}
			return result;
		} catch (BadKeyException e) {
			throw new JolRuntimeException("anti index join failed!", e);
		}
	}

}
