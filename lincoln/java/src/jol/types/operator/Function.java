package jol.types.operator;

import java.util.Set;

import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Table;
import jol.core.Runtime;

public class Function extends Operator {
	
	private Table function;
	
	private Predicate predicate;

	public Function(Runtime context, Table function, Predicate predicate) {
		super(context, predicate.program(), predicate.rule());
		this.function = function;
		this.predicate = predicate;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		try {
			TupleSet result = this.function.insert(tuples, null);
			for (Tuple tuple : result) {
				tuple.schema(predicate.schema().clone());
			}
			return result;
		} catch (UpdateException e) {
			e.printStackTrace();
			throw new P2RuntimeException(e.toString());
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
		return this.function.name() + "(" + predicate + ")";
	}

}
