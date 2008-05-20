package p2.lang.plan;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;

import p2.types.table.Table;
import p2.exec.Query;
import p2.lang.plan.Fact.FactTable;
import p2.lang.plan.Program.ProgramTable;
import p2.lang.plan.Rule.RuleTable;
import p2.lang.plan.Selection.SelectionTable;
import p2.lang.plan.Predicate.PredicateTable;
import p2.lang.plan.Watch.WatchTable;
import p2.lang.plan.Assignment.AssignmentTable;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.PlannerException;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;

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
			super("program", PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
		
		@Override
		protected boolean remove(Tuple tuple) throws UpdateException {
			return super.remove(tuple);
		}
	}
	
	static final ProgramTable    program    = new ProgramTable();
	static final RuleTable       rule       = new RuleTable();
	static final WatchTable      watch      = new WatchTable();
	static final FactTable       fact       = new FactTable();
	static final PredicateTable  predicate  = new PredicateTable();
	static final SelectionTable  selection  = new SelectionTable();
	static final AssignmentTable assignment = new AssignmentTable();
	
	private String name;
	
	private String owner;
	
	private Set<Table> definitions;
	
	private Hashtable<String, Set<Query>> queries;
	
	private Hashtable<String, TupleSet> facts;
	
	private Hashtable<String, Table> tables;
	
	public Program(String name, String owner) {
		this.name        = name;
		this.owner       = owner;
		this.definitions = new HashSet<Table>();
		this.queries     = new Hashtable<String, Set<Query>>();
		this.facts       = new Hashtable<String, TupleSet>();
		this.tables      = new Hashtable<String, Table>();
		try {
			program.force(new Tuple(program.name(), name, owner, this));
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public String toString() {
		String program = "PROGRAM " + this.name + "\n";
		program += "\n============= PROGRAM FACTS ================\n";
		for (TupleSet factSet : facts.values()) {
			for (Tuple fact : factSet) {
				program += fact.toString() + "\n";
			}
		}
		program += "\n============= PROGRAM QUERIES ==============\n";
		for (String input : queries.keySet()) {
			for (Query query : queries.get(input)) {
				program += query.toString() + "\n";
			}
		}
		return program;
	}
	
	public void definition(Table table) {
		this.definitions.add(table);
	}
	
	public void plan() throws PlannerException {
		this.queries.clear();
		this.facts.clear();
		this.tables.clear();
		
		/* First plan out all the rules. */
		TupleSet rules = rule.secondary().get(
				new Key(RuleTable.Field.PROGRAM.ordinal())).lookup(
						new Key.Value(this.name));
		
		for (Tuple tuple : rules) {
			Rule rule = (Rule) tuple.value(RuleTable.Field.OBJECT.ordinal());
			Table table = Table.table(rule.head().name());
			System.err.println("CHECK TABLE " + table.name());
			if (!table.isEvent() && !this.tables.containsKey(table.name())) {
				System.err.println("\tIS TABLE");
				this.tables.put(table.name(), table); // Cache all hard tables that a written.
			}
			else {
				System.err.println("\tIS EVENT");
			}
			
			/* Store all planned queries from a given rule. 
			 * NOTE: delta rules can produce > 1 query. */
			for (Query query : rule.query()) {
				if (!queries.containsKey(query.input().name())) {
					queries.put(query.input().name(), new HashSet<Query>());
				}
				queries.get(query.input().name()).add(query);
			}
		}
		
		/* Accumulate all facts. */
		TupleSet facts = fact.secondary().get(
				new Key(FactTable.Field.PROGRAM.ordinal())).lookup(
						new Key.Value(this.name));
		
		for (Tuple tuple : facts) {
			Tuple fact = (Tuple) tuple.value(FactTable.Field.TUPLE.ordinal());
			if (!this.facts.containsKey(fact.name())) {
				this.facts.put(fact.name(), new TupleSet(fact.name()));
			}
			this.facts.get(fact.name()).add(fact);
		}
		
		/* TODO Register all watch statements. */
	}

	public int compareTo(Program o) {
		return this.name.compareTo(o.name);
	}

	public Hashtable<String, Set<Query>> queries() {
		return this.queries;
	}

	public Hashtable<String, Table> tables() {
		return this.tables;
	}
	
	public Hashtable<String, TupleSet> facts() {
		return this.facts;
	}

	public String name() {
		return this.name;
	}
}
