package p2.types.operator;

import java.util.List;
import java.util.Set;
import java.util.Vector;

import p2.lang.plan.Program;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.PlannerException;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;


public abstract class Operator implements Comparable<Operator> {
	
	private static Long id = new Long(0);
	private static String newID() {
		String identifier = "Operator:" + Operator.id.toString();
		Operator.id += 1L;
		return identifier;
	}
	
	public static class OperatorTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(2);
		
		public enum Field {PROGRAM, RULE, ID, SELECTIVITY, OBJECT};
		public static final Class[] SCHEMA = { 
			String.class,   // Program name
			String.class,   // Rule name
			String.class,   // Operator identifier
			Float.class,    // Selectivity
			Operator.class  // Operator object
		};

		public OperatorTable() {
			super(new TableName(GLOBALSCOPE, "operator"), PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
	}
	
	private final static OperatorTable table = new OperatorTable();
	
	private final String identifier;
	
	protected String program;
	
	protected String rule;
	
	public Operator(String program, String rule) {
		this.identifier = Operator.newID();
		this.program = program;
		this.rule = rule;
		try {
			Tuple me = new Tuple(Operator.table.name(), program, rule, 
							     this.identifier, null, this);
			Operator.table.insert(me);
		} catch (UpdateException e) {
			e.printStackTrace();
			System.exit(0);
		}
	}
	
	public int compareTo(Operator o) {
		return this.identifier.compareTo(o.identifier);
	}
	
	@Override
	public abstract String toString();
	
	public abstract TupleSet evaluate(TupleSet tuples) throws P2RuntimeException;

	public abstract Schema schema(Schema input);
	
	public abstract Set<Variable> requires();
}
