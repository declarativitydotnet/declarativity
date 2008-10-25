package jol.types.operator;

import java.util.Iterator;

import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.table.Table;
import jol.core.Runtime;

/**
 * An AntiScanJoin operator is the forms basis of the 'notin' operator.
 * The input tuples are joined with an inner table and only those 
 * tuples from the input that DO NOT satisfy the join are passed 
 * to the output TupleSet.
 *
 * NOTE: The output schema of this operator will be the same
 * as the input schema.
 */
public class AntiScanJoin extends Join {

	/** The inner relation. */
	private Table table;
	
	/** 
	 * Create a new operator.
	 * @param context The runtime context.
	 * @param predicate The (notin) predicate.
	 * @param input The input schema.
	 */
	public AntiScanJoin(Runtime context, Predicate predicate, Schema input) {
		super(context, predicate, input);
		this.table = context.catalog().table(predicate.name());
	}

	@Override
	public String toString() {
		return "ANTI NEST LOOP JOIN: PREDICATE[" + this.predicate  + "]";
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		TupleSet result = new TupleSet();
		for (Tuple outer : tuples) {
			boolean success = false;
			Iterator<Tuple> it = this.table.tuples();
			while(it.hasNext()) {
				Tuple inner = it.next();
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
