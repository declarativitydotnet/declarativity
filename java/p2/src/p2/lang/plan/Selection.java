package p2.lang.plan;

import java.util.Set;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;

public class Selection extends Term {
	
	public static class SelectionTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public enum Field {PROGRAM, RULE, POSITION, OJBECT};
		public static final Class[] SCHEMA = {
			String.class,   // Program name
			String.class,   // Rule name
			Integer.class,  // Term position
			Selection.class // Selection object
		};

		public SelectionTable() {
			super("selection", PRIMARY_KEY, new TypeList(SCHEMA));
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
	
	
	private Boolean predicate;
	
	public Selection(Boolean predicate) {
		this.predicate = predicate;
		assert(predicate.type() == java.lang.Boolean.class);
	}
	
	@Override
	public String toString() {
		return predicate.toString();
	}

	@Override
	public Set<Variable> requires() {
		return predicate.variables();
	}
}
