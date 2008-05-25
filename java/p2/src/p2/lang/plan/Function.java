package p2.lang.plan;

import java.util.Set;

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

public class Function extends Term {
	
	public enum Field{PROGRAM, RULE, POSITION, NAME, PREDICATE, OBJECT};
	public static class TableFunction extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1,2);
		
		public static final Class[] SCHEMA =  {
			String.class,    // program name
			String.class,    // rule name
			Integer.class,   // position
			String.class,    // function name
			String.class,    // predicate name
			Function.class   // function object
		};

		public TableFunction() {
			super("function", PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Function object = (Function) tuple.value(Field.OBJECT.ordinal());
			if (object == null) {
				throw new UpdateException("Predicate object null");
			}
			object.program   = (String) tuple.value(Field.PROGRAM.ordinal());
			object.rule      = (String) tuple.value(Field.RULE.ordinal());
			object.position  = (Integer) tuple.value(Field.POSITION.ordinal());
			return super.insert(tuple);
		}
	}
	
	private Table function;
	
	private Predicate predicate;
	
	public Function(Table function, Predicate predicate) {
		this.function = function;
		this.predicate = predicate;
	}

	@Override
	public Operator operator() {
		return new p2.types.operator.Function(function, predicate);
	}

	@Override
	public Set<Variable> requires() {
		return this.predicate.requires();
	}

	@Override
	public void set(String program, String rule, Integer position)
			throws UpdateException {
		Program.tfunction.force(new Tuple(Program.tfunction.name(), program, rule, position, 
				                          function.name(), predicate.name(), this));
	}

	@Override
	public String toString() {
		return function.name() + "(" + predicate + ")";
	}
	
	public Predicate predicate() {
		return this.predicate;
	}

}
