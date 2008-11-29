package jol.types.operator;

import java.util.Set;

import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.core.Runtime;

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
			System.exit(0);
		}
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		try {
			TupleSet result = this.function.insert(tuples, null);
			for (Tuple tuple : result) {
				tuple.schema(predicate.schema().clone());
			}
			return result;
		} catch (UpdateException e) {
			e.printStackTrace();
			throw new JolRuntimeException(e.toString());
		}
	}

	@Override
	public Set<Variable> requires() {
		return predicate.requires();
	}

	@Override
	public Schema schema() {
		return predicate.schema().clone();
	}

	@Override
	public String toString() {
		return this.function == null || this.predicate == null ? 
				"null" : this.function.name() + "(" + predicate + ")";
	}

}
