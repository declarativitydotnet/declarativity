package p2.types.operator;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Simple;
import p2.types.basic.TupleSet;
import java.util.Set;

public class ScanJoin extends Operator {
	
	private Predicate predicate;
	
	public ScanJoin(String ID, Predicate predicate) {
		super(ID);
		this.predicate = predicate;
	}
	
	@Override
	public Intermediate evaluate(TupleSet tuples) {
		TupleSet result = new Simple("Operator " + id(), schema((VariableSchema)tuples.schema()));
		return result;
	}

	@Override
	public VariableSchema schema(VariableSchema input) {
		return predicate.schema().join(input);
	}

	@Override
	public Set<Variable> requires() {
		return null;
	}

}
