package p2.lang.plan;

import java.util.Set;
import p2.lang.Compiler;
import p2.lang.plan.Predicate.Field;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.operator.Operator;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;
import p2.types.table.TableName;

public class Function extends Predicate {
	
	public enum Field{PROGRAM, RULE, POSITION, NAME, OBJECT};
	public static class TableFunction extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1,2);
		
		public static final Class[] SCHEMA =  {
			String.class,    // program name
			String.class,    // rule name
			Integer.class,   // position
			String.class,    // function name
			Function.class   // function object
		};

		public TableFunction() {
			super(new TableName(GLOBALSCOPE, "function"), PRIMARY_KEY, new TypeList(SCHEMA));
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
	public Operator operator(Schema input) {
		return new p2.types.operator.Function(function, this);
	}

	@Override
	public void set(String program, String rule, Integer position)
			throws UpdateException {
		super.set(program, rule, position);
		Compiler.tfunction.force(new Tuple(program, rule, position, 
				                          function.name(), this));
	}

	@Override
	public String toString() {
		return function.name() + "(" + super.toString() + ")";
	}
	
}
