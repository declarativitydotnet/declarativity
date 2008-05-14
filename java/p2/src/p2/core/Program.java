package p2.core;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;
import p2.exec.Query;
import p2.lang.plan.Fact;
import p2.lang.plan.Rule;
import p2.lang.plan.Watch;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.EventTable;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

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
	
	private String name;
	
	private List<Table> definitions;
	
	private List<Watch> watches;
	
	private List<Fact> facts;
	
	private List<Rule> rules;
	
	private Hashtable<String, Set<Query>> queries;
	
	private List<Table> references;
	
	public Program(String name) {
		this.name        = name;
		this.definitions = new ArrayList<Table>();
	}
	
	@Override
	public String toString() {
		return this.name;
	}
	
	public int compareTo(Program o) {
		return this.name.compareTo(o.name);
	}
	
	public String name() {
		return this.name;
	}
	
	public Hashtable<String, Set<Query>> queries() {
		return this.queries;
	}
	
	public Hashtable<String, Table> tables() {
		return null;
	}
	
	public Set<String> plan() {
		Set<String> tuples = new HashSet<String>();
		
		return tuples;
	}
	
	public void definition(Table t) {
		definitions.add(t);
	}
	


}
