package p2.types.operator;

import java.util.List;
import java.util.Set;
import java.util.Vector;
import p2.lang.plan.Program;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.ElementException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;


public abstract class Operator {
	
	public static class OperatorTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(2);
		
		public enum Field {PROGRAM, RULE, ID, SELECTIVITY, PRIORITY, OBJECT};
		public static final Class[] SCHEMA = { 
			String.class,   // Program name
			String.class,   // Rule name
			String.class,   // Operator identifier
			Float.class,    // Selectivity
			Integer.class,  // Position priority
			Operator.class  // Operator object
		};

		public OperatorTable() {
			super("operator", PRIMARY_KEY, new TypeList(SCHEMA));
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
	
	public abstract TupleSet evaluate(TupleSet tuples);

	public abstract Schema schema(Schema input);
	
	public abstract Set<Variable> requires();
}
