package p2.lang.plan;

import p2.lang.plan.Rule.RuleTable;
import p2.types.basic.Tuple;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Schema;

public class Selection extends Term {
	
	public static class SelectionTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public static final Schema SCHEMA = 
			new Schema(new Schema.Entry("ProgramName",  String.class),
					   new Schema.Entry("RuleName",     String.class),
					   new Schema.Entry("Position",     Integer.class),
					   new Schema.Entry("Selection",    Selection.class));

		public SelectionTable(Name name, Schema schema, Integer size, Number lifetime, Key key) {
			super(name, schema, key);
		}
		
		@Override
		protected Tuple insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
		
		@Override
		protected boolean remove(Tuple tuple) throws UpdateException {
			return super.remove(tuple);
		}
	}
	
	
	private Expression predicate;
	
	public Selection(Expression predicate) {
		this.predicate = predicate;
		assert(predicate.type() == java.lang.Boolean.class);
	}
	
	@Override
	public String toString() {
		return predicate.toString();
	}
}
