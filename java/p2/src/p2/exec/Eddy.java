package p2.exec;

import java.util.List;

import p2.types.basic.TupleSet;
import p2.types.operator.Operator;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Schema;

public class Eddy {

	public static class Eddies extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1,2);
		
		public static final Schema SCHEMA = 
			new Schema(new Schema.Entry("Program",   String.class),
					   new Schema.Entry("Rule",      String.class),
					   new Schema.Entry("TupleName", String.class),
					   new Schema.Entry("Eddy",      Eddy.class));
		
		protected Eddies(Name name, Schema schema, Integer size, Number lifetime, Key key) {
			super(name, schema, key);
		}
	}
	
	private String input;
	
	private Operator head;
	
	private List<Operator> body;

	public Eddy(String input, Operator head, List<Operator> body) {
		this.input = input;
		this.head = head;
		this.body = body;
	}
	
	public TupleSet evaluate(TupleSet input) {
		
		return null;
	}
}
