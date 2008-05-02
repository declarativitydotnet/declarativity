package p2.types.operator;

import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.SimpleTupleSet;
import p2.types.basic.TupleSet;
import p2.types.function.Filter;

public class Selection extends Operator {
	
	private Set<Variable> requires;
	
	private Filter filter;

	public Selection(String ID, Filter filter) {
		super(ID);
		this.requires = filter.requires();
		this.filter = filter;
	}

	@Override
	public Intermediate evaluate(Intermediate tuples) {
		TupleSet result =  new SimpleTupleSet("Operator " + id(), schema((VariableSchema)tuples.schema()));
		
		return result;
	}

	@Override
	public VariableSchema schema(VariableSchema input) {
		return input;
	}

	@Override
	public Set<Variable> requires() {
		return this.requires;
	}

}
