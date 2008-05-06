package p2.types.operator;

import java.util.HashSet;
import java.util.List;
import java.util.Set;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.TupleSet;

public class Aggregate extends Operator {
	
	private Schema output;
	
	private Set<Variable> requires;

	public Aggregate(String ID, Schema output, Variable aggregate, List<Variable> groupBy) {
		super(ID);
		this.output = output;
		this.requires = new HashSet<Variable>();
		this.requires.add(aggregate);
		this.requires.addAll(groupBy);
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Schema schema(Schema input) {
		return output;
	}

	@Override
	public Set<Variable> requires() {
		return this.requires;
	}

}
