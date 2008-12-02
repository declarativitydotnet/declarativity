package jol.exec;

import java.util.List;
import jol.lang.plan.Predicate;
import jol.types.basic.Tuple;
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
		String query = "QUERY RULE: " + rule();
		return query;
	}

	@Override
	public TupleSet evaluate(TupleSet input) throws JolRuntimeException {
		if (!input.name().equals(input().name())) {
			throw new JolRuntimeException("Query expects event " + input().name() +
					                     " but got event " + input.name());
		}

		TupleSet tuples = new TupleSet(input.name());
		for (Tuple tuple : input) {
			tuple = tuple.clone();
			tuple.schema(input().schema().clone());
			tuples.add(tuple);
		}

		for (Operator oper : body) {
			try {
				tuples = (TupleSet) oper.evaluate(tuples);
			} catch (Throwable t) {
				String error = "ERROR: " + t.toString();
				error += ". Query " + toString() + ". At operator " + oper;
				throw new JolRuntimeException(error, t);
			}
		}

		return tuples;
	}
}
