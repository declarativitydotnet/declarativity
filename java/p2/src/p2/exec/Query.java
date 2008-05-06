package p2.exec;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

public abstract class Query implements Comparable<Query> {

	public static class QueryTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1,2);
		
		public enum Field{PROGRAM, DELETE, INPUT, OUTPUT, OBJECT};
		public static final Class[] SCHEMA = {
			String.class,  // Program name
			String.class,  // Rule name 
			Boolean.class, // Delete rule/query?
			String.class,  // Input tuple name
			String.class,  // Output tuple name
			Query.class    // The query object
		};
		
		public QueryTable() {
			super("query", PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		protected Tuple insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
	}
	
	public Query(String program, String rule, Boolean delete, String input, String output) {
		try {
			p2.core.System.query().insert(
					new Tuple(p2.core.System.query().name(), program, rule, 
							  delete, input, output, this));
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public int compareTo(Query q) {
		return this.hashCode() < q.hashCode() ? -1 :
					this.hashCode() > q.hashCode() ? 1 : 0;
	}
	
	public abstract TupleSet evaluate(TupleSet input);
}
