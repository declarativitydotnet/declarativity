package p2.exec;

import p2.lang.plan.Predicate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

public abstract class Query implements Comparable<Query> {

	public static class QueryTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key();
		
		public enum Field{PROGRAM, RULE, DELETE, EVENT, INPUT, OUTPUT, OBJECT};
		public static final Class[] SCHEMA = {
			String.class,  // Program name
			String.class,  // Rule name
			Boolean.class, // Delete rule/query?
			String.class,  // Event modifier
			String.class,  // Input tuple name
			String.class,  // Output tuple name
			Query.class    // The query object
		};
		
		public QueryTable() {
			super("query", PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		protected boolean insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
	}
	
	private String program;
	
	private String rule;
	
	private Boolean delete;
	
	private Table.Event event;
	
	private Predicate input;
	
	private Predicate output;
	
	public Query(String program, String rule, Boolean delete, Predicate input, Predicate output) {
		this.program = program;
		this.rule = rule;
		this.delete = delete;
		this.event  = input.event();
		this.input = input;
		this.output = output;
		try {
			p2.core.System.query().force(
					new Tuple(p2.core.System.query().name(), program, rule,  delete, 
							  event.toString(), input.name(), output.name(), this));
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public Table.Event event() {
		return this.event;
	}
	
	public abstract String toString();
	
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
	
	public boolean delete() {
		return this.delete;
	}
	
	public Predicate input() {
		return this.input;
	}
	
	public Predicate output() {
		return this.output;
	}
	
	public abstract TupleSet evaluate(TupleSet input) throws P2RuntimeException;
}
