package jol.types.operator;

import java.util.Set;

import jol.core.Runtime;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
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

	/** The output schema. */
	private Schema schema;
	
	private int variablePosition;
	
	private TupleFunction<Comparable> valueFunction;
	
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
		this.schema = input.clone();
		
		if (!this.schema.contains(this.assignment.variable())) {
			this.schema.append(this.assignment.variable());
		}
		this.variablePosition = this.schema.position(assignment.variable().name());
	}

	@Override
	public String toString() {
		return this.assignment == null ? "null" : this.assignment.toString();
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		TupleSet deltas = new TupleSet(tuples.name());
		for (Tuple tuple : tuples) {
			try {
				Comparable[] delta = tuple.toArray();
				// we support appending to the end of the tuple.
				if(variablePosition == delta.length) {
					Comparable[] newDelta = new Comparable[variablePosition+1];
					for(int i = 0; i < delta.length; i++) {
						newDelta[i] = delta[i];
					}
					delta = newDelta;
				}
				delta[variablePosition] = valueFunction.evaluate(tuple);
				deltas.add(new Tuple(delta));
			} catch (Throwable t) {
				System.err.println("SCHEMA " + this.schema);
				String msg = t.toString() + ". Program " + this.assignment.program() +
				             ". Error during assignment " + toString() + 
				             ", on input tuple " + tuple;
				throw new JolRuntimeException(msg, t);
			}
		}
		return deltas;
	}

	@Override
	public Schema schema() {
		return this.schema;
	}

	@Override
	public Set<Variable> requires() {
		return this.assignment.requires();
	}
}
