package jol.lang.plan;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import jol.core.Runtime;
import jol.exec.Query;
import jol.lang.plan.Rule.RuleTable;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.BadKeyException;
import jol.types.exception.PlannerException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;

public class Program implements Comparable<Program> {
	
	public static class ProgramTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "program");
		public static final Key PRIMARY_KEY = new Key(0);
		
		public enum Field{PROGRAM, OWNER, OBJECT};
		public static final Class[] SCHEMA =  {
			String.class,  // Program name
			String.class,  // Program owner
			Program.class  // Program object
		};

		public ProgramTable(Runtime context) {
			super(context, new TableName(GLOBALSCOPE, "program"), PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
		
		@Override
		protected boolean delete(Tuple tuple) throws UpdateException {
			return super.delete(tuple);
		}
	}
	
	private Runtime context;
	
	private String name;
	
	private String owner;
	
	private Set<Table> definitions;
	
	private Map<TableName, Set<Query>> queries;
	
	public Program(Runtime context, String name, String owner) {
		this.context     = context;
		this.name        = name;
		this.owner       = owner;
		this.definitions = new HashSet<Table>();
		this.queries     = new HashMap<TableName, Set<Query>>();
		try {
			context.catalog().table(ProgramTable.TABLENAME).force(new Tuple(name, owner, this));
		} catch (UpdateException e) {
			e.printStackTrace();
			java.lang.System.exit(1);
		}
	}
	
	public Tuple tuple() {
		return new Tuple(name, this);
	}
	
	public Tuple nameTuple() {
		return new Tuple(name);
	}
	
	@Override
	public String toString() {
		return "PROGRAM " + this.name;
	}
	
	public void definition(Table table) {
		for (Table current : definitions) {
			if (current.name().equals(table.name())) {
				definitions.remove(current);
				break;
			}
		}
		this.definitions.add(table);
	}
	
	public Set<Table> definitions() {
		return this.definitions;
	}
	
	public java.lang.Boolean plan() throws PlannerException {
		this.queries.clear();

		try {
			/* First plan out all the rules. */
			TupleSet rules = context.catalog().table(RuleTable.TABLENAME).secondary().get(
					new Key(RuleTable.Field.PROGRAM.ordinal())).lookupByKey(this.name);
			
			if (rules == null) {
				System.err.println("Warning: Program " + this.name + " does not have any rules!?");
				return true;
			}

			for (Tuple tuple : rules) {
				Rule rule = (Rule) tuple.value(RuleTable.Field.OBJECT.ordinal());

				/* Store all planned queries from a given rule. 
				 * NOTE: delta rules can produce > 1 query. */
				for (Query query : rule.query(context)) {
					Predicate input = query.input();
					if (!queries.containsKey(query.input().name())) {
						queries.put(input.name(), new HashSet<Query>());
					}
					queries.get(input.name()).add(query);
				}
			}

		} catch (BadKeyException e) {
			e.printStackTrace();
			return false;
		}
		return true;
	}

	public int compareTo(Program o) {
		return this.name.compareTo(o.name);
	}

	public Set<Query> queries(TableName name) {
		return this.queries == null || !this.queries.containsKey(name) ? null : this.queries.get(name);
	}

	public String name() {
		return this.name;
	}
	
}

