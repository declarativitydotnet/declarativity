package jol.types.operator;

import java.util.Set;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.P2RuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;
import jol.core.Runtime;

public abstract class Operator implements Comparable<Operator> {
	
	private static Long id = new Long(0);
	private static String newID() {
		String identifier = "Operator:" + Operator.id.toString();
		Operator.id += 1L;
		return identifier;
	}
	
	public static class OperatorTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "operator");
		public static final Key PRIMARY_KEY = new Key(2);
		
		public enum Field {PROGRAM, RULE, ID, SELECTIVITY, OBJECT};
		public static final Class[] SCHEMA = { 
			String.class,   // Program name
			String.class,   // Rule name
			String.class,   // Operator identifier
			Float.class,    // Selectivity
			Operator.class  // Operator object
		};

		public OperatorTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
	}
	
	private final String identifier;
	
	protected Runtime context;
	
	protected String program;
	
	protected String rule;
	
	public Operator(Runtime context, String program, String rule) {
		this.identifier = Operator.newID();
		this.context = context;
		this.program = program;
		this.rule = rule;
		try {
			Tuple me = new Tuple(OperatorTable.TABLENAME, program, rule, 
							     this.identifier, null, this);
			context.catalog().table(OperatorTable.TABLENAME).force(me);
		} catch (UpdateException e) {
			e.printStackTrace();
			System.exit(0);
		}
	}
	
	public int compareTo(Operator o) {
		return this.identifier.compareTo(o.identifier);
	}
	
	public void rule(String rule) {
		this.rule = rule;
	}
	
	@Override
	public abstract String toString();
	
	public abstract TupleSet evaluate(TupleSet tuples) throws P2RuntimeException;

	public abstract Schema schema();
	
	public abstract Set<Variable> requires();
}
