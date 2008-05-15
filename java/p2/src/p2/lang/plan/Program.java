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
	
	public Program(String name, String owner) {
		this.name  = name;
		this.owner = owner;
		this.definitions = new HashSet<Table>();
		try {
			program.force(new Tuple(program.name(), name, owner, this));
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public void definition(Table table) {
		this.definitions.add(table);
	}
	
	public void plan() {
		
	}

	public int compareTo(Program o) {
		return this.name.compareTo(o.name);
	}

	public Hashtable<String, Set<Query>> queries() {
		// TODO Auto-generated method stub
		return null;
	}

	public Hashtable<String, Table> tables() {
		// TODO Auto-generated method stub
		return null;
	}

	public String name() {
		return this.name;
	}
}
