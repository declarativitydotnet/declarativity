package jol.types.operator;

import jol.lang.plan.Predicate;
import jol.types.basic.Schema;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
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
	 * @throws PlannerException 
	 */
	public ScanJoin(Runtime context, Predicate predicate, Schema input) throws PlannerException {
		super(context, predicate, input);
		this.table = context.catalog().table(predicate.name());
	}
	
	@Override
	public String toString() {
		return "scan join " + this.table;
	}
	
	@Override
	public TupleSet evaluate(TupleSet outer) throws JolRuntimeException {
		return join(outer, (TupleSet) this.table.tuples());
	}

}
