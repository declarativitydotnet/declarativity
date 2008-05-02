package p2.exec;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Schema;
import p2.types.table.Table;

public abstract class Query implements Comparable<Query> {

	public static class QueryTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1,2);
		
		public static final Schema SCHEMA = 
			new Schema(new Schema.Entry("Program",   String.class),
					   new Schema.Entry("Rule",      String.class),
					   new Schema.Entry("Delete",    Boolean.class),
					   new Schema.Entry("Input",     String.class),
					   new Schema.Entry("Output",    String.class),
					   new Schema.Entry("Query",     Query.class));
		
		protected QueryTable(Name name, Schema schema, Integer size, Number lifetime, Key key) {
			super(name, schema, key);
		}
		
		protected Tuple insert(Tuple t) throws UpdateException {
			return super.insert(t);
		}

	}
	
	private static QueryTable queryTable;

	public Query(String program, String rule, Boolean delete, String input, String output) {
		try {
			queryTable.insert(new Tuple(queryTable.name().toString(), program, rule, delete, input, output, this));
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public int compareTo(Query q) {
		return this.hashCode() < q.hashCode() ? -1 :
					this.hashCode() > q.hashCode() ? 1 : 0;
	}
	
	public static void initialize() {
		try {
			queryTable = (QueryTable) Table.create(new Table.Name("query",QueryTable.class.getName()), 
					                               QueryTable.SCHEMA, QueryTable.PRIMARY_KEY);
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public abstract TupleSet evaluate(TupleSet input);
}
