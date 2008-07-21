package p2.exec;

import java.util.List;
import p2.lang.plan.Predicate;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.operator.Operator;
import p2.types.operator.Projection;
import p2.types.operator.ScanJoin;
import p2.types.table.Aggregation;
import p2.types.table.Table;
import p2.types.table.TableName;

public class BasicQuery extends Query {
	
	private Aggregation aggregation;
	
	private List<Operator> body;

	public BasicQuery(String program, String rule, Boolean isPublic, Boolean isDelete,
					  Predicate event, Predicate head, List<Operator> body) {
		super(program, rule, isPublic, isDelete, event, head);
		this.aggregation = null;
		this.body        = body;
	}
	
	@Override
	public String toString() {
		String query = "Basic Query Rule " + rule() + 
		               ": input " + input().toString();
		if (body != null) {
			for (Operator oper : body) {
				query += " -> " + oper.toString();
			}
		}
		query += " -> " + output().toString();
		return query;
	}

	@Override
	public TupleSet evaluate(TupleSet input) throws P2RuntimeException {
		if (!input.name().equals(input().name())) {
			throw new P2RuntimeException("Query expects input " + input().name() + 
					                     " but got input tuples " + input.name());
		}
		
		TupleSet tuples = new TupleSet(input.name());
		for (Tuple tuple : input) {
			tuple = tuple.clone();
			tuple.schema(input().schema().clone());
			tuples.add(tuple);
		}
		
		for (Operator oper : body) {
			tuples = (TupleSet) oper.evaluate(tuples);
		}
		
		return tuples;
	}

}
