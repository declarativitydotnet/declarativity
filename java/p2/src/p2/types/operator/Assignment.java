package p2.types.operator;

import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.function.TupleFunction;

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
	public TupleSet evaluate(TupleSet tuples) {
		for (Tuple tuple : tuples) {
			tuple.value(variable, this.function.evaluate(tuple));
		}
		return tuples;
	}

	public Schema schema(Schema input) {
		input = new Schema(input);
		if (input.contains(this.variable)) {
			return input;
		}
		input.append(this.variable);
		return input;
	}

	@Override
	public Set<Variable> requires() {
		return requires;
	}
	
}
