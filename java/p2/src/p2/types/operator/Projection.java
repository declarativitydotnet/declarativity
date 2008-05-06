package p2.types.operator;

import java.util.HashSet;
import java.util.List;
import java.util.Set;
import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;


public class Projection extends Operator {
	
	private Predicate predicate;
	
	public Projection(String ID, Predicate predicate) {
		super(ID);
		this.predicate = predicate;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) {
		TupleSet result = new TupleSet(tuples.name());
		for (Tuple tuple : tuples) {
			result.add(tuple.project(this.predicate.schema()));
		}
		return result;
	}

	@Override
	public Schema schema(Schema input) {
		return this.predicate.schema();
	}

	@Override
	public Set<Variable> requires() {
		return this.predicate.requires();
	}
	
	public Predicate predicate() {
		return this.predicate;
	}

}
