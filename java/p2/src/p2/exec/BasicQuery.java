package p2.exec;

import java.util.List;
import p2.lang.plan.Predicate;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.operator.Aggregation;
import p2.types.operator.Operator;
import p2.types.operator.Projection;

public class BasicQuery extends Query {
	
	private Aggregation aggregation;
	
	private Projection head;
	
	private List<Operator> body;

	public BasicQuery(String program, String rule, Boolean delete,
					  Predicate input, Projection head, List<Operator> body) {
		super(program, rule, delete, input, head.predicate());
		this.aggregation = null;
		this.head        = head;
		this.body        = body;
	}
	
	public BasicQuery(String program, String rule, Boolean delete,
			  Predicate input, Projection head, List<Operator> body, 
			  Aggregation aggregation) {
		super(program, rule, delete, input, head.predicate());
		this.aggregation = aggregation;
		this.head        = head;
		this.body        = body;
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
		
		for (Tuple tuple : input) {
			tuple.schema(input().schema());
		}
		
		for (Operator oper : body) {
			input = (TupleSet) oper.evaluate(input);
		}
		
		input = head.evaluate(input);
		return aggregation == null ? input : aggregation.evaluate(input);
	}

}
