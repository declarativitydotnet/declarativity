package p2.types.operator;

import p2.types.basic.OrderedTupleSet;
import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.function.BooleanExpression;
import p2.types.table.Schema;

public class Selection extends Operator {
	
	private BooleanExpression filter;

	public Selection(String program, String rule, String id, BooleanExpression filter) {
		super(program, rule, id);
		this.filter = filter;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) {
		TupleSet result = tuples.sort() == null ? 
				new SimpleTupleSet() : new OrderedTupleSet(tuples.sort());
		
		for (Tuple tuple : tuples) {
			if (filter.eval(tuple)) {
				result.add(tuple);
			}
		}
		return result;
	}

	@Override
	public Schema schema(Schema input) {
		return input;
	}

}
