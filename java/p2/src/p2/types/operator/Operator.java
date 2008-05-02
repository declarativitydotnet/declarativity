package p2.types.operator;

import java.util.List;
import java.util.Set;
import java.util.Vector;

import p2.lang.plan.Program;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.ElementException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Schema;
import p2.types.table.Table.Name;


public abstract class Operator {
	
	public static class OperatorTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(2);
		
		public static final Schema SCHEMA = 
			new Schema(new Schema.Entry("Program",     String.class),
					   new Schema.Entry("Rule",        String.class),
					   new Schema.Entry("ID",          String.class),
					   new Schema.Entry("Selectivity", Float.class),
					   new Schema.Entry("Priority",    Integer.class),
					   new Schema.Entry("Order",       List.class),
					   new Schema.Entry("Operator",    Operator.class));

		public OperatorTable(Name name, Schema schema, Integer size, Number lifetime, Key key) {
			super(name, schema, key);
			// TODO Auto-generated constructor stub
		}
		
	}
	
	private String ID;
	
	private Integer priority;
	
	public Operator(String ID) {
		this.ID = ID;
	}
	
	public String id() {
		return this.ID;
	}
	
	public Integer priority() {
		return this.priority;
	}
	
	public void priority(Integer priority) {
		this.priority = priority;
	}
	
	public abstract Intermediate evaluate(Intermediate tuples);

	public abstract VariableSchema schema(VariableSchema input);
	
	public abstract Set<Variable> requires();
}
