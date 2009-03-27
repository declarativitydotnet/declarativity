package jol.types.operator;

import jol.types.basic.Schema;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;
import jol.core.Runtime;

/**
 * Selection operator applies a selection predicate to
 * the input tuples sending any tuples that pass the predicate
 * to the output tuple set.
 *
 */
public class Selection extends Operator {

	/** The selection predicate. */
	private jol.lang.plan.Selection selection;

	private TupleFunction<java.lang.Boolean> filter;

	/** Create a new selection operator.
	 * @param context The runtime context.
	 * @param selection The selection predicate.
	 * @param input The input schema
	 * @throws PlannerException
	 */
	public Selection(Runtime context, jol.lang.plan.Selection selection, Schema input)
	throws PlannerException {
		super(context, selection.program(), selection.rule());
		this.selection = selection;
		this.filter = selection.predicate().function(input);
	}

	@Override
	public String toString() {
		return "SELECTION [" + this.selection + "]";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		TupleSet result = new BasicTupleSet();
		for (Tuple tuple : tuples) {
			try {
				if (java.lang.Boolean.TRUE.equals(filter.evaluate(tuple))) {
					result.add(tuple);
				}
			} catch (Throwable t) {
				String msg = "ERROR " + t.toString() +
				               ". Program " + this.selection.program() +
				               ". Exception while evaluating selection predicate " +
			 	               this + ". Input tuple: " + tuple;
				throw new JolRuntimeException(msg, t);
			}
		}
		return result;
	}
}
