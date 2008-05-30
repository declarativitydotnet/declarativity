package p2.types.operator;

import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.Table;

public class Function extends Operator {
	
	private Table function;
	
	private Predicate predicate;

	public Function(Table function, Predicate predicate) {
		super(predicate.program(), predicate.rule());
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
	public Schema schema(Schema input) {
		return predicate.schema().clone();
	}

	@Override
	public String toString() {
		return this.function.name() + "(" + predicate + ")";
	}

}
