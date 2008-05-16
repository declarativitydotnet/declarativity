package p2.types.operator;

import p2.lang.plan.Predicate;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Table;

public class AntiScanJoin extends Join {

	private Table table;
	
	public AntiScanJoin(Predicate predicate) {
		super(predicate);
		this.table = Table.table(predicate.name());
	}

	@Override
	public String toString() {
		return "ANTI NEST LOOP JOIN: PREDICATE[" + this.predicate  + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) {
		TupleSet result = new TupleSet(tuples.name() + 
				                       " ANTIJOIN " + 
				                       predicate.name());
		for (Tuple outer : tuples) {
			boolean success = false;
			for (Tuple inner : this.table) {
				Tuple join = outer.join(inner);
				if (join != null) {
					success = true;
					break;
				}
			}
			if (!success) result.add(outer);
		}
		return result;
	}

}
