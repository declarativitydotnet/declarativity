package jol.types.operator;

import java.util.Set;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.TupleSet;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
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

	/** The output schema. NOTE: same as the input schema. */
	private Schema schema;

	/** Create a new selection operator.
	 * @param context The runtime context.
	 * @param selection The selection predicate.
	 * @param input The input schema
	 */
	public Selection(Runtime context, jol.lang.plan.Selection selection, Schema input) {
		super(context, selection.program(), selection.rule());
		this.selection = selection;
		this.schema = input.clone();
		for (Variable var : this.selection.requires()) {
			if (this.schema.contains(var)) {
				var.position(this.schema.position(var.name()));
			}
			else {
				throw new RuntimeException("Selection " + selection + 
						". Error: variable " + var + 
						" not in input schema " + input);
			}
		}
	}

	@Override
	public String toString() {
		return "SELECTION [" + this.selection + "]";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		TupleSet result = new TupleSet(tuples.name());
		TupleFunction<java.lang.Boolean> filter = this.selection.predicate().function();
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

	@Override
	public Schema schema() {
		return this.schema;
	}

	@Override
	public Set<Variable> requires() {
		return this.selection.predicate().variables();
	}
}
