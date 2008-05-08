package p2.lang.plan;

import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

public class Predicate extends Term implements Iterable<Expression> {
	public enum EventModifier{NONE, INSERT, DELETE};
	
	public static class PredicateTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public static final Class[] SCHEMA =  {
			String.class,   // program name
			String.class,   // rule name
			String.class,   // event modifier
			Integer.class,  // position
			Predicate.class // predicate object
		};

		public PredicateTable() {
			super("predicate", PRIMARY_KEY, new TypeList(SCHEMA));
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
	
	private EventModifier event;
	
	private List<Expression> arguments;
	
	private Schema schema;
	
	public Predicate(boolean notin, String name, EventModifier event, List<Expression> arguments) {
		this.table = null;
		this.notin = notin;
		this.name = name;
		this.event = event;
		this.arguments = arguments;
	}
	
	public Schema schema() {
		return schema;
	}
	
	public void table(Table table) {
		this.table = table;
	}
	
	public Table table() {
		return this.table;
	}
	
	public boolean notin() {
		return this.notin;
	}
	
	public EventModifier event() {
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
