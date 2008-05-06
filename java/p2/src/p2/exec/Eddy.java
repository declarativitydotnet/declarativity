package p2.exec;

import java.util.List;

import p2.types.basic.Schema;
import p2.types.basic.TupleSet;
import p2.types.operator.Operator;
import p2.types.table.Key;
import p2.types.table.ObjectTable;

public class Eddy extends Query {

	private String input;
	
	private Operator head;
	
	private List<Operator> body;

	public Eddy(String program, String rule, Boolean delete, String input, String output, 
			    Operator head, List<Operator> body) {
		super(program, rule, delete, input, output);
		this.input = input;
		this.head = head;
		this.body = body;
	}
	
	public TupleSet evaluate(TupleSet input) {
		
		return null;
	}
}
