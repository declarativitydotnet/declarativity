package p2.exec;

import java.util.List;
import p2.lang.plan.Predicate;
import p2.types.basic.TupleSet;
import p2.types.exception.RuntimeException;
import p2.types.operator.Operator;
import p2.types.operator.Projection;

public class BasicQuery extends Query {
	
	private Projection head;
	
	private List<Operator> body;

	public BasicQuery(String program, String rule, Boolean delete,
					  Predicate input, Projection head, List<Operator> body) {
		super(program, rule, delete, input, head.predicate());
		this.head = head;
		this.body = body;
	}
	
	@Override
	public String toString() {
		String query = "Basic Query Rule + " + rule() + 
		               ": input " + input().toString();
		for (Operator oper : body) {
			query += " -> " + oper.toString();
		}
		query += " -> " + head.toString();
		return query;
	}

	@Override
	public TupleSet evaluate(TupleSet input) throws RuntimeException {
		assert(input.name().equals(input.name()));
		
		for (Operator oper : body) {
			input = (TupleSet) oper.evaluate(input);
		}
		return (TupleSet) head.evaluate(input);
	}

}
