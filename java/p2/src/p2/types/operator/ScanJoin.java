package p2.types.operator;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.function.Filter;
import p2.types.table.Table;

import java.util.List;
import java.util.Set;

public class ScanJoin extends Join {
	
	private Table table;
	
	private List<Filter> filters;
	
	public ScanJoin(Predicate predicate) {
		super(predicate);
		this.table = Table.table(predicate.name());
		this.filters = super.filters();
	}
	
	@Override
	public String toString() {
		return "NEST LOOP JOIN: PREDICATE[" + this.predicate  + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet(tuples.name() + 
				                       " JOIN " + 
				                       predicate.name());
		for (Tuple outer : tuples) {
			for (Tuple inner : this.table) {
				if (satisfyFilters(inner)) {
					inner.schema(this.predicate.schema());
					Tuple join = outer.join(result.name(), inner);
					if (join != null) {
						result.add(join);
					}
				}
			}
		}
		return result;
	}
	
	private boolean satisfyFilters(Tuple tuple) throws P2RuntimeException {
		for (Filter filter : filters) {
			if (filter.evaluate(tuple) == false) {
				return false;
			}
		}
		return true;
	}


}
