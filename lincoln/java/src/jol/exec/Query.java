package jol.exec;

import jol.lang.plan.Predicate;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.P2RuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;
import jol.core.Runtime;

public abstract class Query implements Comparable<Query> {

	public static class QueryTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "query");
		public static final Key PRIMARY_KEY = new Key();
		
		public enum Field{PROGRAM, RULE, PUBLIC, DELETE, EVENT, INPUT, OUTPUT, OBJECT};
		public static final Class[] SCHEMA = {
			String.class,     // Program name
			String.class,     // Rule name
			Boolean.class,    // Public query?
			Boolean.class,    // Delete rule/query?
			String.class,     // Event modifier
			TableName.class,  // Input table name
			TableName.class,  // Output table name
			Query.class       // The query object
		};
		
		public QueryTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		protected boolean insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
	}
	
	private String program;
	
	private String rule;
	
	private Boolean isPublic;
	
	private Boolean isDelete;
	
	private Predicate.Event event;
	
	private Predicate input;
	
	private Predicate output;
	
	public Query(Runtime context, String program, String rule, Boolean isPublic, Boolean isDelete, 
			     Predicate input, Predicate output) {
		this.program = program;
		this.rule = rule;
		this.isPublic = isPublic;
		this.isDelete = isDelete;
		this.event  = input.event();
		this.input = input;
		this.output = output;
		try {
			context.catalog().table(QueryTable.TABLENAME).force(
					new Tuple(program, rule, isPublic, isDelete, 
							  event.toString(), input.name(), output.name(), this));
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public Predicate.Event event() {
		return this.event;
	}
	
	public abstract String toString();
	
	public boolean equals(Object o) {
		return o instanceof Query &&
				compareTo((Query)o) == 0;
	}
	
	public int compareTo(Query q) {
		return this.hashCode() < q.hashCode() ? -1 :
					this.hashCode() > q.hashCode() ? 1 : 0;
	}
	
	public String program() {
		return this.program;
	}
	
	public String rule() {
		return this.rule;
	}
	
	public boolean isDelete() {
		return this.isDelete;
	}
	
	public boolean isPublic() {
		return this.isPublic;
	}
	
	public Predicate input() {
		return this.input;
	}
	
	public Predicate output() {
		return this.output;
	}
	
	public abstract TupleSet evaluate(TupleSet input) throws P2RuntimeException;
}
