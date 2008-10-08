package p2.types.operator;

import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;
import p2.core.Runtime;

public class Assign extends Operator {
	
	private p2.lang.plan.Assignment assignment;
	
	private Schema schema;
	
	public Assign(Runtime context, p2.lang.plan.Assignment assignment, Schema input) {
		super(context, assignment.program(), assignment.rule());
		this.assignment = assignment;
		this.schema = input.clone();
		
		if (!this.schema.contains(this.assignment.variable())) {
			this.schema.append(this.assignment.variable());
		}
	}
	
	@Override
	public String toString() {
		return this.assignment.toString();
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		Variable variable = assignment.variable();
		TupleFunction<Comparable> function = assignment.value().function();
		TupleSet deltas = new TupleSet(tuples.name());
		for (Tuple tuple : tuples) {
			Tuple delta = tuple.clone();
			delta.value(variable, function.evaluate(delta));
			deltas.add(delta);
		}
		return deltas;
	}
	
	@Override
	public Schema schema() {
		return this.schema;
	}

	@Override
	public Set<Variable> requires() {
		return this.assignment.requires();
	}
	
}
