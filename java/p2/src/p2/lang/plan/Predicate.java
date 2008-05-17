package p2.lang.plan;

import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import p2.lang.plan.Selection.SelectionTable.Field;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.operator.AntiScanJoin;
import p2.types.operator.Operator;
import p2.types.operator.ScanJoin;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

public class Predicate extends Term implements Iterable<Expression> {
	public enum EventModifier{NONE, INSERT, DELETE};
	
	public enum Field{PROGRAM, RULE, POSITION, EVENT, OBJECT};
	public static class PredicateTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1,2);
		
		public static final Class[] SCHEMA =  {
			String.class,     // program name
			String.class,     // rule name
			Integer.class,    // position
			String.class,     // Event
			Predicate.class   // predicate object
		};

		public PredicateTable() {
			super("predicate", PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Predicate object = (Predicate) tuple.value(Field.OBJECT.ordinal());
			if (object == null) {
				throw new UpdateException("Predicate object null");
			}
			object.program   = (String) tuple.value(Field.PROGRAM.ordinal());
			object.rule      = (String) tuple.value(Field.RULE.ordinal());
			object.position  = (Integer) tuple.value(Field.POSITION.ordinal());
			return super.insert(tuple);
		}
		
		@Override
		protected boolean remove(Tuple tuple) throws UpdateException {
			return super.remove(tuple);
		}
	}
	
	private boolean notin;
	
	private String name;
	
	private EventModifier event;
	
	private Arguments arguments;
	
	private Schema schema;
	
	public Predicate(boolean notin, String name, EventModifier event, List<Expression> arguments) {
		super();
		this.notin = notin;
		this.name = name;
		this.event = event;
		this.arguments = new Arguments(this, arguments);
	}
	
	public Schema schema() {
		return schema;
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

	/**
	 * An iterator over the predicate arguments.
	 */
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

	@Override
	public Operator operator() {
		// TODO Add code that searches for an index join
		
		if (notin) {
			return new AntiScanJoin(this);
		}
		return new ScanJoin(this);
	}

	@Override
	public void set(String program, String rule, Integer position) {
		Tuple me = new Tuple(Program.predicate.name(), program, rule, position, event.toString(), this);
		try {
			Program.predicate.force(me);
		} catch (UpdateException e) {
			e.printStackTrace();
		}
		
		this.schema = new Schema(name());
		for (Expression arg : this) {
			if (arg instanceof Variable) {
				this.schema.append((Variable) arg);
			}
			else {
				this.schema.append(new DontCare(arg.type()));
			}
		}
	}
	
}
