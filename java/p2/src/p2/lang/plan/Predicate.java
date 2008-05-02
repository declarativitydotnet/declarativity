package p2.lang.plan;

import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import p2.types.basic.Tuple;
import p2.types.exception.UpdateException;
import p2.types.operator.VariableSchema;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Schema;

public class Predicate extends Term implements Iterable<Expression> {
	
	public static class PredicateTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public static final Schema SCHEMA = 
			new Schema(new Schema.Entry("ProgramName",  String.class),
					   new Schema.Entry("RuleName",     String.class),
					   new Schema.Entry("Position",     Integer.class),
					   new Schema.Entry("Predicate",    Predicate.class));

		public PredicateTable(Name name, Schema schema, Key key) {
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
	
	private Table table;
	
	private boolean notin;
	
	private String name;
	
	private String event;
	
	private List<Expression> arguments;
	
	private VariableSchema schema;
	
	public Predicate(boolean notin, String name, String event, List<Expression> arguments) {
		this.table = null;
		this.notin = notin;
		this.name = name;
		this.event = event;
		this.arguments = arguments;
	}
	
	public VariableSchema schema() {
		return schema;
	}
	
	public void table(Table table) {
		this.table = table;
	}
	
	public p2.types.table.Table table() {
		return this.table.object();
	}
	
	public boolean notin() {
		return this.notin;
	}
	
	public String event() {
		return this.event;
	}
	
	public String name() {
		return this.name;
	}

	public Iterator<Expression> iterator() {
		return this.arguments.iterator();
	}
	
	@Override
	public String toString() {
		String value = (notin ? "notin " : "") + name + "(";
		if (arguments.size() == 0) {
			return value + ")";
		}
		value += arguments.get(0).toString();
		for (int i = 1; i < arguments.size(); i++) {
			value += ", " + arguments.get(i);
		}
		return value + ")";
	}

	@Override
	public Set<Variable> requires() {
		Set<Variable> variables = new HashSet<Variable>();
		for (Expression arg : arguments) {
			if (!Variable.class.isAssignableFrom(arg.getClass())) {
				variables.addAll(arg.variables());
			}
		}
		return variables;
	}
	
}
