package jol.lang.plan;

import java.util.Set;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;
import jol.types.operator.Operator;
import jol.types.operator.Assign;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;
import jol.core.Runtime;

public class Assignment extends Term {
	
	public static class AssignmentTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "assignment");
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public enum Field {PROGRAM, RULE, POSITION, OBJECT};
		public static final Class[] SCHEMA = { 
			String.class,    // Program name
			String.class,    // Rule name
			Integer.class,   // Rule position
			Assignment.class // Assignment object
		};

		public AssignmentTable(Runtime context) {
			super(context, new TableName(GLOBALSCOPE, "assignment"), PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Assignment object = (Assignment) tuple.value(Field.OBJECT.ordinal());
			if (object == null) {
				throw new UpdateException("Assignment object null");
			}
			object.program   = (String) tuple.value(Field.PROGRAM.ordinal());
			object.rule      = (String) tuple.value(Field.RULE.ordinal());
			object.position  = (Integer) tuple.value(Field.POSITION.ordinal());
			return super.insert(tuple);
		}
		
		@Override
		protected boolean delete(Tuple tuple) throws UpdateException {
			return super.delete(tuple);
		}
	}
	
	private Variable variable;
	
	private Expression value;
	
	public Assignment(Variable variable, Expression value) {
		this.variable = variable;
		this.value = value;
	}
	
	@Override
	public String toString() {
		return variable.toString() + " := " + value.toString();
	}

	@Override
	public Set<Variable> requires() {
		return value.variables();
	}
	
	public Variable variable() {
		return this.variable;
	}
	
	public Expression value() {
		return this.value;
	}

	@Override
	public Operator operator(Runtime context, Schema input) {
		return new Assign(context, this, input);
	}

	@Override
	public void set(Runtime context, String program, String rule, Integer position) throws UpdateException {
		context.catalog().table(AssignmentTable.TABLENAME).force(new Tuple(program, rule, position, this));
	}

}
