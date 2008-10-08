package jol.types.operator;

import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.table.Table;
import jol.core.Runtime;

public class ScanJoin extends Join {
	
	public Table table;
	
	public ScanJoin(Runtime context, Predicate predicate, Schema input) {
		super(context, predicate, input);
		this.table = context.catalog().table(predicate.name());
	}
	
	@Override
	public String toString() {
		return "NEST LOOP JOIN: PREDICATE[" + this.predicate  + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet();
		TupleSet innerTuples = this.table.tuples();
		for (Tuple outer : tuples) {
			for (Tuple inner : innerTuples) {
				inner.schema(this.predicate.schema().clone());
				
				if (validate(outer, inner)) {
					Tuple join = outer.join(inner);
					if (join != null) {
						result.add(join);
					}
				}
			}
		}
		return result;
	}

}
