package jol.lang.plan;

import jol.core.Runtime;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.exception.UpdateException;
import jol.types.operator.Operator;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;

public class Function extends Predicate {
	
	public enum Field{PROGRAM, RULE, POSITION, NAME, OBJECT};
	public static class TableFunction extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "function");
		public static final Key PRIMARY_KEY = new Key(0,1,2);
		
		public static final Class[] SCHEMA =  {
			String.class,    // program name
			String.class,    // rule name
			Integer.class,   // position
			String.class,    // function name
			Function.class   // function object
		};

		public TableFunction(Runtime context) {
			super(context, new TableName(GLOBALSCOPE, "function"), PRIMARY_KEY, SCHEMA);
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Function object = (Function) tuple.value(Field.OBJECT.ordinal());
			if (object == null) {
				throw new UpdateException("Predicate object null");
			}
			object.position  = (Integer) tuple.value(Field.POSITION.ordinal());
			return super.insert(tuple);
		}
	}
	
	private Table function;
	
	public Function(Table function, Predicate predicate) {
		super(predicate);
		this.function = function;
	}

	@Override
	public Operator operator(Runtime context, Schema input) {
		return new jol.types.operator.Function(context, function, this);
	}

	@Override
	public void set(Runtime context, String program, String rule, Integer position)
			throws UpdateException {
		super.set(context, program, rule, position);
		context.catalog().table(TableFunction.TABLENAME).force(new Tuple(program, rule, position, 
				                          function.name(), this));
	}

	@Override
	public String toString() {
		return function.name() + "(" + super.toString() + ")";
	}
	
}
