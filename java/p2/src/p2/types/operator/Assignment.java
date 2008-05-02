package p2.types.operator;

import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.OrderedTupleSet;
import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.function.TupleFunction;
import p2.types.table.Schema;

public class Assignment extends Operator {
	
	private Variable variable;
	
	private Set<Variable> requires;
	
	/* The function to evaluate. */
	private TupleFunction<Comparable> function;
	
	public Assignment(String ID, Variable variable, Set<Variable> requires, TupleFunction<Comparable> function) {
		super(ID);
		this.variable = variable;
		this.function = function;
	}

	@Override
	public Intermediate evaluate(Intermediate tuples) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VariableSchema schema(VariableSchema input) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<Variable> requires() {
		return requires;
	}
	
}
