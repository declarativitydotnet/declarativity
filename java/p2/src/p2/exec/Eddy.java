package p2.exec;

import java.util.List;

import p2.lang.plan.Predicate;
import p2.types.basic.Schema;
import p2.types.basic.TupleSet;
import p2.types.operator.Operator;
import p2.types.operator.Projection;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

public class Eddy extends Query {

	private Projection head;
	
	private List<Operator> body;

	public Eddy(String program, String rule, Boolean delete,
			    Predicate input, Projection output,  List<Operator> body) {
		super(program, rule, delete, input, output.predicate());
		this.head = output;
		this.body = body;
	}
	
	@Override
	public String toString() {
		String query = "EDDY: input " + input().toString();
		for (Operator oper : body) {
			query += " -> " + oper.toString();
		}
		query += " -> " + head.toString();
		return query;
	}
	
	public TupleSet evaluate(TupleSet input) {
		
		return null;
	}
}
