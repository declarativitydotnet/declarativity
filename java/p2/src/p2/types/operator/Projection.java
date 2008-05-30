package p2.types.operator;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;


public class Projection extends Operator {
	
	private Predicate predicate;
	
	List<TupleFunction<Comparable>> accessors = new ArrayList<TupleFunction<Comparable>>();
	
	public Projection(Predicate predicate) {
		super(predicate.program(), predicate.rule());
		this.predicate = predicate;
		
		for (p2.lang.plan.Expression arg : predicate) {
			accessors.add(arg.function());
		}
	}
	
	@Override
	public String toString() {
		return "PROJECTION PREDICATE[" + predicate + "]";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet(predicate.name());
		for (Tuple tuple : tuples) {
			List<Comparable> values = new ArrayList<Comparable>();
			for (TupleFunction<Comparable> accessor : accessors) {
				values.add(accessor.evaluate(tuple));
			}
			Tuple projection = new Tuple(values);
			projection.schema(predicate.schema());
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
