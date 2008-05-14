package p2.types.operator;

import java.util.Set;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.TupleSet;
import p2.types.basic.Tuple;
import p2.types.exception.RuntimeException;
import p2.types.function.Filter;
import p2.types.function.TupleFunction;

public class Selection extends Operator {
	
	private p2.lang.plan.Selection selection;
	
	public Selection(p2.lang.plan.Selection selection) {
		super(selection.program(), selection.rule(), selection.position());
		this.selection = selection;
	}

	@Override
	public String toString() {
		return "SELECTION [" + this.selection + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) throws RuntimeException {
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
	public Schema schema(Schema input) {
		return new Schema(input);
	}

	@Override
	public Set<Variable> requires() {
		return this.selection.predicate().variables();
	}

}
