package jol.exec;

import java.util.List;
import jol.lang.plan.Predicate;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.operator.Operator;
import jol.core.Runtime;

/**
 * Implements a query using a fixed chain of operators.
 */
public class BasicQuery extends Query {
	private List<Operator> body;

	public BasicQuery(Runtime context, String program, String rule,
			          Boolean isPublic, Boolean isAsync, Boolean isDelete,
					  Predicate event, Predicate head, List<Operator> body) {
		super(context, program, rule, isPublic, isAsync, isDelete, event, head);
		this.body = body;
	}

	@Override
	public String toString() {
		return "QUERY RULE: " + rule();
	}

	@Override
	public TupleSet evaluate(TupleSet input) throws JolRuntimeException {
		TupleSet tuples = new BasicTupleSet();
		for (Tuple tuple : input) {
			tuple = tuple.clone();
			tuples.add(tuple);
		}

		for (Operator oper : this.body) {
			try {
				tuples = oper.evaluate(tuples);
			} catch (Throwable t) {
				String error = "ERROR: " + t.toString();
				error += ". Query " + toString() + ". At operator " + oper;
				throw new JolRuntimeException(error, t);
			}
		}

		return tuples;
	}
}
