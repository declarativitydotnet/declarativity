package p2.lang.plan;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;

import p2.types.table.Table;
import p2.core.Clock;
import p2.exec.Query;
import p2.lang.plan.Fact.FactTable;
import p2.lang.plan.Function.TableFunction;
import p2.lang.plan.Program.ProgramTable;
import p2.lang.plan.Rule.RuleTable;
import p2.lang.plan.Selection.SelectionTable;
import p2.lang.plan.Predicate.PredicateTable;
import p2.lang.plan.Watch.WatchTable;
import p2.lang.plan.Assignment.AssignmentTable;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.PlannerException;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.TableName;
import p2.lang.Compiler;

public class Program implements Comparable<Program> {
	
	public static class ProgramTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0);
		
		public enum Field{PROGRAM, OWNER, OBJECT};
		public static final Class[] SCHEMA =  {
			String.class,  // Program name
			String.class,  // Program owner
			Program.class  // Program object
		};

		public ProgramTable() {
			super(new TableName(GLOBALSCOPE, "program"), PRIMARY_KEY, new TypeList(SCHEMA));
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
	
	private String name;
	
	private String owner;
	
	private Set<Table> definitions;
	
	private Hashtable<TableName, Set<Query>> queries;
	
	private TupleSet periodics;
	
	public Program(String name, String owner) {
		this.name        = name;
		this.owner       = owner;
		this.definitions = new HashSet<Table>();
		this.queries     = new Hashtable<TableName, Set<Query>>();
		this.periodics   = new TupleSet(p2.core.System.periodic().name());
		try {
			Compiler.programs.force(new Tuple(name, owner, this));
			p2.core.System.program(name, this);
		} catch (UpdateException e) {
			e.printStackTrace();
			java.lang.System.exit(1);
		}
	}
	
	public Tuple tuple() {
		return new Tuple(name, this);
	}
	
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
		this.periodics.clear();

		try {
			/* First plan out all the rules. */
			TupleSet rules = Compiler.rule.secondary().get(
					new Key(RuleTable.Field.PROGRAM.ordinal())).lookup(this.name);
			
			if (rules == null) {
				System.err.println("Warning: Program " + this.name + " does not have any rules!?");
				return true;
			}

			for (Tuple tuple : rules) {
				Rule rule = (Rule) tuple.value(RuleTable.Field.OBJECT.ordinal());

				/* Store all planned queries from a given rule. 
				 * NOTE: delta rules can produce > 1 query. */
				for (Query query : rule.query(this.periodics)) {
					Predicate input = query.input();
					if (!queries.containsKey(query.input().name())) {
						queries.put(input.name(), new HashSet<Query>());
					}
					queries.get(input.name()).add(query);
				}
				
			}

			if (periodics.size() > 0) {
				for (Tuple tuple : this.periodics) {
					p2.core.System.periodic().force(tuple);
				}
			}
		} catch (BadKeyException e) {
			e.printStackTrace();
			return false;
		} catch (UpdateException e) {
			e.printStackTrace();
			return false;
		}
		return true;
	}

	public int compareTo(Program o) {
		return this.name.compareTo(o.name);
	}

	public Set<Query> queries(TableName name) {
		return this.queries.get(name);
	}

	public TupleSet periodics() {
		return this.periodics;
	}

	public String name() {
		return this.name;
	}
	
}

