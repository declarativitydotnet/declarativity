package jol.types.operator;

import java.util.Set;

import jol.core.Runtime;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

/**
 * An Assign operator assigns some constant value to some attribute position
 * in the input tuples. The output tuples will contain the assigned value.
 *
 */
public class Assign<C extends Comparable<C> > extends Operator {
	
	/** The assignment predicate. */
	private jol.lang.plan.Assignment assignment;
	
	/** The output schema. */
	private Schema schema;
	
	/**
	 * Create a new assign operator.
	 * @param context The runtime context.
	 * @param assignment The assignment predicate.
	 * @param input The input schema.
	 */
	public Assign(Runtime context, jol.lang.plan.Assignment assignment, Schema input) {
		super(context, assignment.program(), assignment.rule());
		this.assignment = assignment;
		this.schema = input.clone();
		
		if (!this.schema.contains(this.assignment.variable())) {
			this.schema.append(this.assignment.variable());
		}
	}
	
	@Override
	public String toString() {
		return this.assignment == null ? "null" : this.assignment.toString();
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		Variable variable = assignment.variable();
		TupleFunction<C> function = assignment.value().function();
		TupleSet deltas = new TupleSet(tuples.name());
		for (Tuple tuple : tuples) {
			Tuple delta = tuple.clone();
			delta.value(variable, function.evaluate(delta));
			deltas.add(delta);
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
