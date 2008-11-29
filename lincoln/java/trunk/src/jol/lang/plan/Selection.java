package jol.lang.plan;

import java.util.Set;

import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;
import jol.types.operator.Operator;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;
import jol.core.Runtime;

public class Selection extends Term {
	
	public static class SelectionTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "selection");
		public static final Key PRIMARY_KEY = new Key(0,1,2);
		
		public enum Field {PROGRAM, RULE, POSITION, OBJECT};
		public static final Class[] SCHEMA = {
			String.class,   // Program name
			String.class,   // Rule name
			Integer.class,  // Term position
			Selection.class // Selection object
		};

		public SelectionTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Selection object = (Selection) tuple.value(Field.OBJECT.ordinal());
			if (object == null) {
				throw new UpdateException("Selection object null!");
			}
			object.program  = (String) tuple.value(Field.PROGRAM.ordinal());
			object.rule     = (String) tuple.value(Field.RULE.ordinal());
			object.position = (Integer) tuple.value(Field.POSITION.ordinal());
			return super.insert(tuple);
		}
	}
	
	private Boolean<?> predicate;
	
	public Selection(Boolean predicate) {
		super();
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
	
	public Boolean predicate() {
		return this.predicate;
	}

	@Override
	public Operator operator(Runtime context, Schema input) {
		return new jol.types.operator.Selection(context, this, input);
	}

	@Override
	public void set(Runtime context, String program, String rule, Integer position) throws UpdateException {
		context.catalog().table(SelectionTable.TABLENAME).force(new Tuple(program, rule, position, this));
	}
}
