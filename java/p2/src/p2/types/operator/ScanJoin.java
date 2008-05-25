package p2.types.operator;

import p2.lang.plan.Predicate;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.table.Table;

public class ScanJoin extends Join {
	
	private Table table;
	
	public ScanJoin(Predicate predicate) {
		super(predicate);
		this.table = Table.table(predicate.name());
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
			for (Tuple inner : this.table.tuples()) {
				inner.schema(this.predicate.schema());
				
				if (validate(outer, inner)) {
					Tuple join = outer.join(result.name(), inner);
					if (join != null) {
						result.add(join);
					}
				}
			}
		}
		return result;
	}

}
