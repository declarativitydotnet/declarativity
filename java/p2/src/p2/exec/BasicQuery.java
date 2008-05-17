package p2.exec;

import java.util.List;
import p2.lang.plan.Predicate;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
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
		String query = "Basic Query Rule " + rule() + 
		               ": input " + input().toString();
		for (Operator oper : body) {
			query += " -> " + oper.toString();
		}
		query += " -> " + head.toString();
		return query;
	}

	@Override
	public TupleSet evaluate(TupleSet input) throws P2RuntimeException {
		assert(input.name().equals(input.name()));
		System.err.println("Evaluate Query " + rule() + " input predicate " + input().toString());
		
		for (Tuple tuple : input) {
			tuple.schema(input().schema());
		}
		
		for (Operator oper : body) {
			System.err.println("\tApply operator " + oper + " input tuple set = " + input);
			input = (TupleSet) oper.evaluate(input);
			System.err.println("\tRESULT " + input);
		}
		System.err.println("\tApply operator " + head);
		
		input = head.evaluate(input);
		System.err.println("FINAL RESULT " + input);
		return input;
	}

}
