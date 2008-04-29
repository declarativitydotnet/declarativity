package p2.types.operator;

import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Simple;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.Schema;

public class IndexJoin extends Operator {
	
	private Predicate predicate;
	
	private Index index;
	
	private Key lookup;

	public IndexJoin(String ID, Predicate predicate, Index index, Key lookup) {
		super(ID);
		this.predicate = predicate;
		this.index = index;
		this.lookup = lookup;
	}
	
	@Override
	public Intermediate evaluate(TupleSet tuples) {
		TupleSet result = new Simple("Operator " + id(), schema((VariableSchema)tuples.schema()));
		
		return result;
	}

	@Override
	public Set<Variable> requires() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VariableSchema schema(VariableSchema input) {
		return predicate.schema().join(input);
	}

}
