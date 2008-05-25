package p2.types.operator;

import p2.lang.plan.Predicate;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
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
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet(tuples.name() + 
				                       " ANTIJOIN " + 
				                       predicate.name());
		for (Tuple outer : tuples) {
			boolean success = false;
			for (Tuple inner : this.table.tuples()) {
				inner.schema(this.predicate.schema());
				if (!validate(outer, inner)) {
					success = true;
					break;
				}
				else {
					Tuple join = outer.join(result.name(), inner);
					if (join != null) {
						/*
						System.err.println("ANTIJOIN SCHEMA: outer = " + outer.schema() + " inner = " + inner.schema());
						System.err.println("ANTIJOIN TUPLE: outer = " + outer + " inner = " + inner);
						System.err.println("ANTIJOIN RESULT: " + join);
						*/
						success = true;
						break;
					}
				}
			}
			if (!success) result.add(outer);
		}
		return result;
	}

}
