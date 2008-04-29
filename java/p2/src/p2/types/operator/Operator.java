package p2.types.operator;

import java.util.Vector;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.ElementException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Schema;
import p2.types.table.Table.Name;


public abstract class Operator {
	
	public static class OperatorTable extends ObjectTable {

		protected OperatorTable(Name name, Schema schema, Integer size, Number lifetime, Key key) {
			super(name, schema, key);
			// TODO Auto-generated constructor stub
		}
		
	}
	
	
	private String program;
	
	private String rule;
	
	private String id;

	public Operator(String program, String rule, String id) {
		this.program = program;
		this.rule = rule;
		this.id = id;
	}
	
	public abstract TupleSet evaluate(TupleSet tuples);

	public abstract Schema schema(Schema input);
}
