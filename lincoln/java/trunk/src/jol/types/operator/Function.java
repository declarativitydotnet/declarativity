package jol.types.operator;

import jol.core.Runtime;
import jol.lang.plan.Predicate;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;

/**
 * Applies a table function operator to the input stream.
 */
public class Function extends Operator {

	/** The table function. */
	private Table function;

	/** The predicate used as argument to the table function. */
	private Predicate predicate;

	/**
	 * Create a new table function operator.
	 * @param context The runtime context.
	 * @param function The table function.
	 * @param predicate The predicate argument.
	 */
	public Function(Runtime context, Table function, Predicate predicate) {
		super(context, predicate.program(), predicate.rule());
		this.function = function;
		this.predicate = predicate;
		if (this.function == null || this.predicate == null) {
			System.err.println("TRIED TO CREATE A TABLE FUNCTION WITH NULL ARGUMENTS");
			System.exit(-1);
		}
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		try {
			return this.function.insert(tuples, null);
		} catch (UpdateException e) {
			e.printStackTrace();
			throw new JolRuntimeException(e.toString());
		}
	}

	@Override
	public String toString() {
		return this.function == null || this.predicate == null ?
				"null" : this.function.name() + "(" + predicate + ")";
	}

}
