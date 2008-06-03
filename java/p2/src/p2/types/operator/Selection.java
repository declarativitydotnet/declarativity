package p2.types.operator;

import java.util.Set;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.TupleSet;
import p2.types.basic.Tuple;
import p2.types.exception.P2RuntimeException;
import p2.types.function.Filter;
import p2.types.function.TupleFunction;

public class Selection extends Operator {
	
	private p2.lang.plan.Selection selection;
	
	private Schema schema;
	
	public Selection(p2.lang.plan.Selection selection, Schema input) {
		super(selection.program(), selection.rule());
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
