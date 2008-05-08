package p2.lang.plan;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

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
		protected Tuple insert(Tuple tuple) throws UpdateException {
			return super.insert(tuple);
		}
		
		@Override
		protected boolean remove(Tuple tuple) throws UpdateException {
			return super.remove(tuple);
		}
	}
	
	private String name;
	
	private List<Table> tables;
	
	private List<EventTable> events;
	
	private List<Watch> watches;
	
	private List<Fact> facts;
	
	private List<Rule> rules;
	
	public Program(String name) {
		this.name    = name;
		this.tables  = new ArrayList<Table>();
		this.events  = new ArrayList<EventTable>();
		this.watches = new ArrayList<Watch>();
		this.facts   = new ArrayList<Fact>();
		this.rules   = new ArrayList<Rule>();
	}
	
	@Override
	public String toString() {
		String value = "Program " + name + "\n";
		for (Table t : tables) {
			value += t + "\n";
		}
		for (EventTable e : events) {
			value += e + "\n";
		}
		for (Watch w : watches) {
			value += w + "\n";
		}
		for (Fact f : facts) {
			value += f + "\n";
		}
		for (Rule r : rules) {
			value += r + "\n";
		}
		return value;
	}
	
	public Set<String> plan() {
		Set<String> tuples = new HashSet<String>();
		
		return tuples;
	}
	
	public Tuple evaluate(Tuple eval) {
		
		return null;
	}
	
	public void table(Table t) {
		tables.add(t);
	}
	
	public void event(EventTable e) {
		events.add(e);
	}
	
	public void watch(Watch w) {
		watches.add(w);
	}
	
	public void fact(Fact f) {
		facts.add(f);
	}
	
	public void rule(Rule r) {
		rules.add(r);
	}

	public int compareTo(Program o) {
		return this.name.compareTo(o.name);
	}
}
