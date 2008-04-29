package p2.types.operator;

import p2.lang.plan.Variable;
import p2.types.basic.OrderedTupleSet;
import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.function.Function;
import p2.types.table.Schema;

public class Assignment extends Operator {
	
	private Variable variable;
	
	/* The function to evaluate. */
	private Function<Comparable> function;
	
	public Assignment(String program, String rule, String id, Variable variable, Function<Comparable> function) {
		super(program, rule, id);
		this.function = function;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Schema schema(Schema input) {
		// TODO Auto-generated method stub
		return null;
	}
	
}
