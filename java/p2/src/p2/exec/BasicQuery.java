package p2.exec;

import java.util.List;
import p2.lang.plan.Predicate;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.operator.Operator;
import p2.types.operator.Projection;
import p2.types.table.Aggregation;
import p2.types.table.Table;

public class BasicQuery extends Query {
	
	private Aggregation aggregation;
	
	private Projection head;
	
	private List<Operator> body;

	public BasicQuery(String program, String rule, Boolean delete,
					  Predicate input, Projection head, List<Operator> body) {
		super(program, rule, delete, input, head.predicate());
		if (input.schema() == null) {
			System.err.println("INPUT NULL SCHEMA " + input.name());
			input.schema().size();
		}
		this.aggregation = null;
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
		if (!input.name().equals(input().name())) {
			throw new P2RuntimeException("Query expects input " + input().name() + 
					                     " but got input tuples " + input.name());
		}
		
		for (Tuple tuple : input) {
			tuple.schema(input().schema().clone());
		}
		
		for (Operator oper : body) {
			input = (TupleSet) oper.evaluate(input);
		}
		
		return head.evaluate(input);
	}

}
