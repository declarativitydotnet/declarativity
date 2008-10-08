package jol.types.operator;

import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.core.Runtime;

public class IndexJoin extends Join {
	
	private Key lookupKey;
	
	private Index index;
	
	public IndexJoin(Runtime context, Predicate predicate, Schema input, Key lookupKey, Index index) {
		super(context, predicate, input);
		this.lookupKey = lookupKey;
		this.index = index;
	}
	
	@Override
	public String toString() {
		return "INDEX JOIN PREDICATAE[" + predicate.toString() + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		TupleSet result = new TupleSet();
		for (Tuple outer : tuples) {
			for (Tuple inner : this.index.lookup(this.lookupKey, outer)) {
				if (validate(outer, inner)) {
					inner.schema(this.predicate.schema().clone());
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
