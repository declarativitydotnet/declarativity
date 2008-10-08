package p2.types.operator;

import p2.lang.plan.Predicate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.core.Runtime;

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
