package types.element;

import types.basic.Function;
import types.basic.OrderedTupleSet;
import types.basic.SimpleTupleSet;
import types.basic.Tuple;
import types.basic.TupleSet;

public class Assignment extends Operator {
	
	/* The function to evaluate. */
	private Function<Comparable> function;
	
	/* The field position that will be given the
	 * function return value. */
	private Integer field;

	public Assignment(String id, String name, Function<Comparable> function, Integer field) {
		super(id, name);
		this.function = function;
		this.field = field;
	}
	
	@Override
	public TupleSet simple_action(TupleSet tuples) {
		TupleSet result = tuples.sort() == null ? 
				new SimpleTupleSet() : new OrderedTupleSet(tuples.sort());
		
		for (Tuple tuple : tuples) {
			tuple.value(field, function.eval(tuple));
		}
		return result;
	}

}
