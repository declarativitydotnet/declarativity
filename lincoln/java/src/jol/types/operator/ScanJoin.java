package jol.types.operator;

import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.table.Table;
import jol.core.Runtime;

/**
 * The ScanJoin operator joins the input tuples with 
 * some table along some set of common variable names.
 * The tuples that pass the join are sent to the output.
 */
public class ScanJoin extends Join {
	
	/** The table that the input tuples will be joined along. */
	public Table table;
	
	/**
	 * Create a new ScanJoin operator.
	 * @param context The runtime context.
	 * @param predicate The predicate of the table that will be joined.
	 * @param input The input schema.
	 */
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
