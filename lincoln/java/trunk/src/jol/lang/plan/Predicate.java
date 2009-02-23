package jol.lang.plan;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import jol.core.Runtime;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.exception.PlannerException;
import jol.types.exception.UpdateException;
import jol.types.operator.AntiIndexJoin;
import jol.types.operator.AntiScanJoin;
import jol.types.operator.IndexJoin;
import jol.types.operator.Operator;
import jol.types.operator.ScanJoin;
import jol.types.table.HashIndex;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;

public class Predicate extends Term implements Iterable<Expression> {
	public enum Field{PROGRAM, RULE, POSITION, EVENT, OBJECT};
	public static class PredicateTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "predicate");
		public static final Key PRIMARY_KEY = new Key(0,1,2);

		public static final Class[] SCHEMA =  {
			String.class,     // program name
			String.class,     // rule name
			Integer.class,    // position
			String.class,     // Event
			Predicate.class   // predicate object
		};

		public PredicateTable(Runtime context) {
			super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
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

	public enum Event{NONE, INSERT, DELETE};

	public Runtime context;

	private boolean notin;

	private TableName name;

	private Event event;

	private Arguments arguments;

	public Predicate(Runtime context, boolean notin, TableName name, Event event, List<Expression> arguments) {
		super();
		this.context = context;
		this.notin = notin;
		this.name = name;
		this.event = event;
		this.arguments = new Arguments(this, arguments);
	}

	public Predicate(Runtime context, boolean notin, TableName name, Event event, Schema schema) {
		super();
		this.context = context;
		this.notin = notin;
		this.name = name;
		this.event = event;
		this.arguments = new Arguments(this, schema.variables());
	}

	protected Predicate(Predicate pred) {
		super();
		this.context   = pred.context;
		this.notin     = pred.notin;
		this.name      = pred.name;
		this.event     = pred.event;
		this.arguments = pred.arguments != null ? pred.arguments.clone() : null;
	}

	public Predicate clone() {
		return new Predicate(this);
	}

	public Schema schema() {
		List<Variable> variables = new ArrayList<Variable>();
		for (Expression e : this.arguments) {
			if (e instanceof Variable) {
				variables.add((Variable) e);
			}
			else {
				variables.add(new DontCare(e.node(), e.type()));
			}
		}
		return new Schema(this.name, variables);
	}
	
	@Override
	public Schema schema(Schema input) {
		if (!this.notin) {
			for (Variable var : this.schema().variables()) {
				if (!input.contains(var)) {
					input.append(var);
				}
			}
		}
		return input;
	}

	public boolean notin() {
		return this.notin;
	}

	public Event event() {
		return this.event;
	}

	void event(Event event) {
		this.event = event;
	}

	public TableName name() {
		return this.name;
	}

	public Variable locationVariable() {
		for (Expression argument : this) {
			if (argument instanceof Variable && ((Variable) argument).loc()) {
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
	
	public void remove(Expression e) {
		this.arguments.remove(e);
	}

	public List<Expression> arguments() {
		return this.arguments;
	}

	void arguments(List<Expression> arguments) {
		this.arguments = new Arguments(this, arguments);
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
		for (Expression<?> arg : arguments) {
			if (!(arg instanceof Variable)) {
				variables.addAll(arg.variables());
			}
		}
		return variables;
	}

	@Override
	public Operator operator(Runtime context, Schema input) throws PlannerException {
		/* Determine the join and lookup keys. */
		Key lookupKey = new Key();
		Key indexKey  = new Key();
		Schema schema = schema();
		for (Variable var : schema.variables()) {
			if (input.contains(var)) {
				indexKey.add(schema.position(var.name()));
				lookupKey.add(input.position(var.name()));
			}
		}

		Table table = context.catalog().table(this.name);
		Index index = null;
		if (indexKey.size() > 0) {
			if (table.primary().key().equals(indexKey)) {
				index = table.primary();
			}
			else if (table.secondary().containsKey(indexKey)) {
				index = table.secondary().get(indexKey);
			}
			else {
				index = new HashIndex(context, table, indexKey, Index.Type.SECONDARY);
				table.secondary().put(indexKey, index);
			}
		}
		
		if (notin) {
			if (index != null) {
				return new AntiIndexJoin(context, this, input, lookupKey, index);
			} else {
				return new AntiScanJoin(context, this, input);
			}
		} else {
			if (index != null) {
				return new IndexJoin(context, this, input, lookupKey, index);
			} else {
				return new ScanJoin(context, this, input);
			}
		}
	}

	@Override
	public void set(Runtime context, String program, String rule, Integer position) throws UpdateException {
		context.catalog().table(PredicateTable.TABLENAME)
			.force(new Tuple(program, rule, position, event.toString(), this));
	}

}
