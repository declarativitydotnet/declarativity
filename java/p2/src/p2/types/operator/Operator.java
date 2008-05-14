package p2.types.operator;

import java.util.List;
import java.util.Set;
import java.util.Vector;

import p2.core.Program;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.PlannerException;
import p2.types.exception.RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;


public abstract class Operator implements Comparable<Operator> {
	
	private static Long id = new Long(0);
	private static String newID() {
		String identifier = "Operator:" + Operator.id.toString();
		Operator.id += 1L;
		return identifier;
	}
	
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
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
	}
	
	private final static OperatorTable table = new OperatorTable();
	
	private final String identifier;
	
	public Operator(String program, String rule, Integer position) {
		this.identifier = Operator.newID();
		try {
			Tuple me = new Tuple(Operator.table.name(), program, rule, 
							     this.identifier, null, position, this);
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
	
	public abstract TupleSet evaluate(TupleSet tuples) throws RuntimeException;

	public abstract Schema schema(Schema input);
	
	public abstract Set<Variable> requires();
}
