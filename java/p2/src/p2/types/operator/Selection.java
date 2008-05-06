package p2.types.operator;

import java.util.Set;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.TupleSet;
import p2.types.basic.Tuple;
import p2.types.function.Filter;

public class Selection extends Operator {
	
	private Set<Variable> requires;
	
	private Filter filter;

	public Selection(String ID, Set<Variable> requires, Filter filter) {
		super(ID);
		this.requires = requires;
		this.filter = filter;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) {
		TupleSet result = new TupleSet(tuples.name());
		for (Tuple tuple : tuples) {
			if (this.filter.evaluate(tuple)) {
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
		return this.requires;
	}

}
