package jol.types.operator;

import java.util.Set;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.TupleSet;
import jol.types.basic.Tuple;
import jol.types.exception.P2RuntimeException;
import jol.types.function.TupleFunction;
import jol.core.Runtime;

public class Selection extends Operator {
	
	private jol.lang.plan.Selection selection;
	
	private Schema schema;
	
	public Selection(Runtime context, jol.lang.plan.Selection selection, Schema input) {
		super(context, selection.program(), selection.rule());
		this.selection = selection;
		this.schema = input.clone();
	}

	@Override
	public String toString() {
		return "SELECTION [" + this.selection + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet(tuples.name());
		TupleFunction<java.lang.Boolean> filter = this.selection.predicate().function();
		for (Tuple tuple : tuples) {
			if (java.lang.Boolean.TRUE.equals(filter.evaluate(tuple))) {
				result.add(tuple);
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
