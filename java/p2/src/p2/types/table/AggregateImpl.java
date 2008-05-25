package p2.types.table;
import java.util.Hashtable;
import java.util.List;
import java.util.ArrayList;
import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.function.Aggregate;

public class AggregateImpl extends Table.Aggregate {
	
	private TupleSet tuples;
	
	private List<Integer> groupBy;
	
	private p2.lang.plan.Aggregate aggregate;
	
	private Hashtable<Tuple, Aggregate> aggregates;
	
	public AggregateImpl(Table table, Predicate predicate) {
		super(table);
		assert(table.type() != Table.Type.FUNCTION);
		this.tuples = new TupleSet(predicate.name());
		this.groupBy = new ArrayList<Integer>();
		
		for (p2.lang.plan.Expression arg : predicate) {
			if (arg instanceof p2.lang.plan.Aggregate) {
				this.aggregate = (p2.lang.plan.Aggregate) arg;
			}
			else {
				groupBy.add(arg.position());
			}
		}
	}
	
	@Override
	public TupleSet tuples() {
		return this.tuples.clone();
	}
	
	@Override
	public Class type() {
		return Aggregate.function(this.aggregate).returnType();
	}

	@Override
	public void delete(TupleSet tuples) throws P2RuntimeException {
		/* Reset and recompute all aggregates next time around. */
		aggregates.clear();
		tuples.clear();
		insert(table.tuples());
	}

	@Override
	public void insert(TupleSet tuples) throws P2RuntimeException {
		for (Tuple tuple : tuples) {
			Tuple groupby = groupby(tuple);
			if (!aggregates.containsKey(groupby)) {
				aggregates.put(groupby,  Aggregate.function(this.aggregate));
			}
		    aggregates.get(groupby).evaluate(tuple);
		}
		
		for (p2.types.function.Aggregate function : aggregates.values()) {
			if (!this.tuples.contains(function.result())) {
				this.tuples.add(function.result());
			}
		}
		
		if (table.type() != Table.Type.TABLE) {
			aggregates.clear();
			tuples.clear();
		}
	}
	
	
	private Tuple groupby(Tuple input) throws P2RuntimeException {
		List<Comparable> values = new ArrayList<Comparable>();
		for (Integer position : this.groupBy) {
			values.add(input.value(position));
		}
		return new Tuple("groupby", values);
	}

}
