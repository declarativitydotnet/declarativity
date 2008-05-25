package p2.types.operator;

import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;

public class Assign extends Operator {
	
	private p2.lang.plan.Assignment assignment;
	
	public Assign(p2.lang.plan.Assignment assignment) {
		super(assignment.program(), assignment.rule());
		this.assignment = assignment;
	}
	
	@Override
	public String toString() {
		return this.assignment.toString();
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		Variable variable = assignment.variable();
		TupleFunction<Comparable> function = assignment.value().function();
		for (Tuple tuple : tuples) {
			tuple.value(variable, function.evaluate(tuple));
		}
		return tuples;
	}
	
	public Schema schema(Schema input) {
		input = new Schema(input);
		if (input.contains(this.assignment.variable())) {
			return input;
		}
		input.append(this.assignment.variable());
		return input;
	}

	@Override
	public Set<Variable> requires() {
		return this.assignment.requires();
	}
	
}
