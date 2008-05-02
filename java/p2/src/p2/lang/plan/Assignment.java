package p2.lang.plan;

import java.util.Set;

import p2.types.basic.Tuple;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Schema;

public class Assignment extends Term {
	
	public static class AssignmentTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public static final Schema SCHEMA = 
			new Schema(new Schema.Entry("ProgramName",  String.class),
					   new Schema.Entry("RuleName",     String.class),
					   new Schema.Entry("Position",     Integer.class),
					   new Schema.Entry("Assignment",   Assignment.class));

		public AssignmentTable(Name name, Schema schema, Integer size, Number lifetime, Key key) {
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
	
	private Variable variable;
	
	private Expression value;
	
	public Assignment(Variable variable, Expression value) {
		this.variable = variable;
		this.value = value;
	}
	
	public String toString() {
		return variable.toString() + " := " + value.toString();
	}

	@Override
	public Set<Variable> requires() {
		return value.variables();
	}
}
