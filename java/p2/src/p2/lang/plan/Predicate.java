package p2.lang.plan;

import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import p2.lang.Compiler;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.operator.AntiScanJoin;
import p2.types.operator.IndexJoin;
import p2.types.operator.Operator;
import p2.types.operator.ScanJoin;
import p2.types.table.HashIndex;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;
import p2.types.table.Table;

public class Predicate extends Term implements Iterable<Expression> {
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
			super(new TableName(GLOBALSCOPE, "predicate"), PRIMARY_KEY, new TypeList(SCHEMA));
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
		protected boolean delete(Tuple tuple) throws UpdateException {
			return super.delete(tuple);
		}
	}
	
	private boolean notin;
	
	private TableName name;
	
	private Table.Event event;
	
	private Arguments arguments;
	
	private Schema schema;
	
	public Predicate(boolean notin, TableName name, Table.Event event, List<Expression> arguments) {
		super();
		this.notin = notin;
		this.name = name;
		this.event = event;
		this.arguments = new Arguments(this, arguments);
	}
	
	public Predicate(boolean notin, TableName name, Table.Event event, Schema schema) {
		super();
		this.notin = notin;
		this.name = name;
		this.event = event;
		this.schema = schema;
		this.arguments = new Arguments(this, (List) schema.variables());
	}
	
	protected Predicate(Predicate pred) {
		super();
		this.notin     = pred.notin;
		this.name      = pred.name;
		this.event     = pred.event;
		this.arguments = pred.arguments;
		this.schema    = pred.schema;
	}
	
	public Schema schema() {
		return schema;
	}
	
	public boolean notin() {
		return this.notin;
	}
	
	public Table.Event event() {
		return this.event;
	}
	
	void event(Table.Event event) {
		this.event = event;
	}
	
	public TableName name() {
		return this.name;
	}
	
	public Variable locationVariable() {
		for (Expression argument : this) {
			if (argument instanceof Variable && ((Variable)argument).loc()) {
				return (Variable) argument;
			}
		}
		return null;
	}
	
	public boolean containsAggregation() {
		for (Expression e : arguments) {
			if (e instanceof Aggregate) {
				return true;
			}
		}
		return false;
	}

	/**
	 * An iterator over the predicate arguments.
	 */
	public Iterator<Expression> iterator() {
		return this.arguments.iterator();
	}
	
	public Expression argument(Integer i) {
		return this.arguments.get(i);
	}
	
	public int arguments() {
		return this.arguments.size();
	}
	
	@Override
	public String toString() {
		assert(schema.size() == arguments.size());
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
			if (!(arg instanceof Variable)) {
				variables.addAll(arg.variables());
			}
		}
		return variables;
	}

	@Override
	public Operator operator(Schema input) {
		/* Determine the join and lookup keys. */
		Key lookupKey = new Key();
		Key indexKey  = new Key();
		for (Variable var : this.schema.variables()) {
			if (input.contains(var)) {
				indexKey.add(var.position());
				lookupKey.add(input.variable(var.name()).position());
			}
		}
		
		if (notin) {
			return new AntiScanJoin(this, input);
		}
		
		Table table = Table.table(this.name);
		Index index = null;
		if (indexKey.size() > 0) {
			if (table.primary().key().equals(indexKey)) {
				index = table.primary();
			}
			else if (table.secondary().contains(indexKey)) {
				index = table.secondary().get(indexKey);
			}
			else {
				index = new HashIndex(table, indexKey, Index.Type.SECONDARY);
				table.secondary().put(indexKey, index);
			}
		}
		
		if (index != null) {
			return new IndexJoin(this, input, lookupKey, index);
		}
		else {
			return new ScanJoin(this, input);
		}
	}
	
	@Override
	public void set(String program, String rule, Integer position) throws UpdateException {
		Compiler.predicate.force(new Tuple(program, rule, position, event.toString(), this));
		
		this.schema = new Schema(name());
		for (Expression arg : arguments) {
			if (arg instanceof Variable) {
				this.schema.append((Variable) arg);
			}
			else {
				this.schema.append(new DontCare(arg.type()));
			}
		}
	}
	
}
