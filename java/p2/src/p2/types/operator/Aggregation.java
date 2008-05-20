package p2.types.operator;
import java.util.Hashtable;
import java.util.Set;
import java.util.List;
import java.util.ArrayList;
import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;

public class Aggregation extends Operator {
	
	private Predicate predicate;
	
	private List<TupleFunction<Comparable>> accessors;
	private p2.lang.plan.Aggregate aggregate;
	
	public Aggregation(Predicate predicate) {
		super(predicate.program(), predicate.rule(), predicate.position());
		this.predicate = predicate;
		this.accessors = new ArrayList<TupleFunction<Comparable>>(); 
		this.aggregate = null;
		for (p2.lang.plan.Expression arg : predicate) {
			if (arg instanceof p2.lang.plan.Aggregate) {
				this.aggregate = (p2.lang.plan.Aggregate) arg;
			}
			else {
				accessors.add(arg.function());
			}
		}
	}
	
	@Override
	public String toString() {
		return "Aggregation PREDICATE[ " + this.predicate.toString() + 
		       "] aggregate " + this.aggregate.toString();
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		assert(tuples.name().equals(this.predicate.name()));
		
		Hashtable<Tuple, p2.types.function.Aggregate> functions =
			new Hashtable<Tuple, p2.types.function.Aggregate>();
		for (Tuple tuple : tuples) {
			Tuple groupby = groupby(tuple);
			if (!functions.containsKey(groupby)) {
				functions.put(groupby, 
						      p2.types.function.Aggregate.function(this.aggregate));
			}
		    functions.get(groupby).evaluate(tuple);
		}
		
		TupleSet result = new TupleSet(this.predicate.name());
		for (p2.types.function.Aggregate function : functions.values()) {
			result.add(function.result());
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
	
	
	private Tuple groupby(Tuple input) throws P2RuntimeException {
		List<Comparable> values = new ArrayList<Comparable>();
		for (TupleFunction<Comparable> accessor : accessors) {
			values.add(accessor.evaluate(input));
		}
		return new Tuple("groupby", values);
	}
}
