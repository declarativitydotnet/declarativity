package jol.types.operator;

import jol.core.Runtime;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;

/**
 * An Assign operator assigns some constant value to some attribute position
 * in the input tuples. The output tuples will contain the assigned value.
 */
public class Assign<C extends Comparable<C> > extends Operator {
	/** The assignment predicate. */
	private jol.lang.plan.Assignment assignment;

	private int variablePosition;

	private TupleFunction<Object> valueFunction;

	/**
	 * Create a new assign operator.
	 * @param context The runtime context.
	 * @param assignment The assignment predicate.
	 * @param input The input schema.
	 * @throws PlannerException
	 */
	public Assign(Runtime context, jol.lang.plan.Assignment assignment, Schema input) throws PlannerException {
		super(context, assignment.program(), assignment.rule());
		this.assignment = assignment;
		this.valueFunction = assignment.value().function(input);

		if (input.contains(this.assignment.variable())) {
			this.variablePosition = input.position(assignment.variable().name());
		}
		else {
			this.variablePosition = input.size();
		}
	}

	@Override
	public String toString() {
		return this.assignment == null ? "null" : this.assignment.toString();
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		TupleSet deltas = new BasicTupleSet(tuples.name());
		for (Tuple tuple : tuples) {
			try {
				Object[] delta = tuple.toArray();
				// we support appending to the end of the tuple.
				if (variablePosition == delta.length) {
					Object[] newDelta = new Object[variablePosition+1];
					for (int i = 0; i < delta.length; i++) {
						newDelta[i] = delta[i];
					}
					delta = newDelta;
				}
				delta[variablePosition] = valueFunction.evaluate(tuple);
				deltas.add(new Tuple(delta));
			} catch (Throwable t) {
				String msg = t.toString() + ". Program " + this.assignment.program() +
				             ". Error during assignment " + toString() +
				             ", position " + variablePosition + ", tuple size " + tuple.size() + ". Tuple = " + tuple;
				throw new JolRuntimeException(msg, t);
			}
		}
		return deltas;
	}
}
