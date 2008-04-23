package types.element;

import types.basic.OrderedTupleSet;
import types.basic.SimpleTupleSet;
import types.basic.Tuple;
import types.basic.TupleSet;
import types.function.BooleanExpression;

public class Selection extends Operator {
	
	private BooleanExpression filter;

	public Selection(String id, String name, BooleanExpression filter) {
		super(id, name);
		this.filter = filter;
	}

	@Override
	public TupleSet simple_action(TupleSet tuples) {
		TupleSet result = tuples.sort() == null ? 
				new SimpleTupleSet() : new OrderedTupleSet(tuples.sort());
		
		for (Tuple tuple : tuples) {
			if (filter.eval(tuple)) {
				result.add(tuple);
			}
		}
		return result;
	}

}
