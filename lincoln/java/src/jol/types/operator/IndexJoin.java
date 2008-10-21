package jol.types.operator;

import jol.core.Runtime;
import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.exception.P2RuntimeException;
import jol.types.table.Index;
import jol.types.table.Key;

/**
 * An index join uses an index to reduce the number of tuples
 * scanned along the inner table. 
 */
public class IndexJoin extends Join {
	
	/** The lookup key used to obtain join matches from the inner relation. */
	private Key lookupKey;
	
	/** The index used to perform the inner relation lookup. */
	private Index index;
	
	/**
	 * Create a new index join operator.
	 * @param context The runtime context.
	 * @param predicate The (table) predicate referencing the inner relation.
	 * @param input The input tuple schema.
	 * @param lookupKey The lookup key.
	 * @param index The index.
	 */
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
		try {
			TupleSet result = new TupleSet();
			for (Tuple outer : tuples) {
				for(Tuple inner : this.index.lookupByKey(lookupKey.project(outer))) {
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
		} catch (BadKeyException e) {
			throw new P2RuntimeException("index join failed!", e);
		}
	}
}
