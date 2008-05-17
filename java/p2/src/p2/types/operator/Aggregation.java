package p2.types.operator;
import java.util.Hashtable;
import java.util.Set;
import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;

public class Aggregation extends Operator {
	
	private Predicate predicate;
	
	private Schema groupBy;
	private p2.lang.plan.Aggregate aggregate;
	
	public Aggregation(Predicate predicate) {
		super(predicate.program(), predicate.rule(), predicate.position());
		this.predicate = predicate;
		this.groupBy = new Schema(predicate.name());
		this.aggregate = null;
		for (Variable var : predicate.schema().variables()) {
			if (var instanceof p2.lang.plan.Aggregate) {
				this.aggregate = (p2.lang.plan.Aggregate) var;
			}
			else {
				groupBy.append(var);
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
		Hashtable<Tuple, p2.types.function.Aggregate> functions =
			new Hashtable<Tuple, p2.types.function.Aggregate>();
		for (Tuple tuple : tuples) {
			Tuple projection = tuple.project(this.groupBy);
			if (!functions.containsKey(projection)) {
				functions.put(projection, 
						p2.types.function.Aggregate.function(this.aggregate));
			}
		    functions.get(projection).evaluate(tuple.project(this.predicate.schema()));
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
}
