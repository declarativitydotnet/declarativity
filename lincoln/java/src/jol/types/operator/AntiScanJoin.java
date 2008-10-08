package jol.types.operator;

import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.table.Table;
import jol.core.Runtime;

public class AntiScanJoin extends Join {

	private Table table;
	
	public AntiScanJoin(Runtime context, Predicate predicate, Schema input) {
		super(context, predicate, input);
		this.table = context.catalog().table(predicate.name());
	}

	@Override
	public String toString() {
		return "ANTI NEST LOOP JOIN: PREDICATE[" + this.predicate  + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet();
		for (Tuple outer : tuples) {
			boolean success = false;
			for (Tuple inner : this.table.tuples()) {
				inner.schema(this.predicate.schema());
				if (validate(outer, inner) && outer.join(inner) != null) {
					success = true;
					break;
				}
			}
			if (!success) result.add(outer);
		}
		return result;
	}

}
