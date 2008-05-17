package p2.types.operator;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.function.Filter;
import p2.types.table.Index;
import p2.types.table.Key;

public class IndexJoin extends Join {
	
	private Index index;
	
	private List<String> names;
	
	private List<Filter> filters;
	
	public IndexJoin(Predicate predicate, Index index) {
		super(predicate);
		this.index = index;
		this.names = new ArrayList<String>();
		this.filters = super.filters();
		List<Variable> variables = predicate.schema().variables();
		for (Integer i : index.key()) {
			this.names.add(variables.get(i).name());
		}
		
	}
	
	@Override
	public String toString() {
		return "INDEX JOIN PREDICATAE[" + predicate.toString() + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet("(" + tuples.name() + " join " + predicate.name() + ")");
		for (Tuple outer : tuples) {
			Key key = new Key();
			for (String name : this.names) {
				int position = outer.schema().position(name);
				if (position < 0) {
					throw new P2RuntimeException("Variable " + name + 
							" does not exist in probe tuple schema " + outer.schema());
				}
				key.add(position);
			}
			Key.Value value = key.value(outer);
			TupleSet inner = this.index.lookup(value);
			for (Tuple i : inner) {
				if (satisfyFilters(i)) {
					Tuple join = outer.join(result.name(), i);
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
