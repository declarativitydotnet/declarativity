package jol.types.operator;

import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.table.Table;
import jol.core.Runtime;

/**
 * An AntiScanJoin operator forms the basis of the 'notin' operator.
 * The input tuples are joined with an inner table and only those
 * tuples from the input that DO NOT satisfy the join are passed
 * to the output TupleSet.
 *
 * NOTE: The output schema of this operator will be the same
 * as the input schema.
 */
public class AntiScanJoin extends Join {

	/** The inner relation. */
	private Table table;

	/**
	 * Create a new operator.
	 * @param context The runtime context.
	 * @param predicate The (notin) predicate.
	 * @param input The input schema.
	 * @throws PlannerException
	 */
	public AntiScanJoin(Runtime context, Predicate predicate, Schema input) throws PlannerException {
		super(context, predicate, input);
		this.table = context.catalog().table(predicate.name());
	}

	@Override
	public String toString() {
		return "anti nested-loop join " + table;
	}

	@Override
	public TupleSet evaluate(TupleSet outerTuples) throws JolRuntimeException {
		TupleSet result = new TupleSet();
		for (Tuple outer : outerTuples) {
			TupleSet join = join(outer, table.tuples());
			if (join.size() == 0) result.add(outer);
		}
		return result;
	}
}
