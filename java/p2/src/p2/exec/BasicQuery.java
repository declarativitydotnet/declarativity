package p2.exec;

import java.util.HashSet;
import java.util.List;

import p2.lang.plan.Predicate;
import p2.types.basic.Intermediate;
import p2.types.basic.TupleSet;
import p2.types.operator.Operator;
import p2.types.operator.Projection;

public class BasicQuery extends Query {
	
	private Projection head;
	
	private Predicate input;
	
	private List<Operator> body;

	public BasicQuery(String program, String rule, Boolean delete,
					  Projection head, Predicate input, List<Operator> body) {
		super(program, rule, delete, head.predicate().name(), input.name());
		this.head = head;
		this.input = input;
		this.body = body;
	}

	@Override
	public TupleSet evaluate(TupleSet input) {
		assert(input.name().equals(input.name()));
		
		for (Operator oper : body) {
			input = (TupleSet) oper.evaluate(input);
		}
		return (TupleSet) head.evaluate(input);
	}

}
