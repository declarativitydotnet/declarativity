package p2.types.operator;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.OrderedTupleSet;
import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Schema;


public class Projection extends Operator {
	
	private Predicate predicate;
	
	private List<Variable> variables;

	public Projection(String ID, Predicate predicate, List<Variable> variables) {
		super(ID);
		this.predicate = predicate;
		this.variables = variables;
	}

	@Override
	public Intermediate evaluate(Intermediate tuples) {
		TupleSet result = new SimpleTupleSet("Operator " + id(), schema((VariableSchema)tuples.schema()));
		
		return result;
	}

	@Override
	public VariableSchema schema(VariableSchema input) {
		return predicate.schema().project(input, variables);
	}

	@Override
	public Set<Variable> requires() {
		return new HashSet<Variable>(variables);
	}
	
	public Predicate predicate() {
		return this.predicate;
	}

}
