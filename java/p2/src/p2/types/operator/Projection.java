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
import p2.types.exception.P2RuntimeException;


public class Projection extends Operator {
	
	private Predicate predicate;
	
	public Projection(Predicate predicate) {
		super(predicate.program(), predicate.rule(), predicate.position());
		this.predicate = predicate;
	}
	
	@Override
	public String toString() {
		return "PROJECTION PREDICATE[" + predicate.name() + "]";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet(predicate.name());
		for (Tuple tuple : tuples) {
			Tuple projection = tuple.project(this.predicate.schema());
			result.add(projection);
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
