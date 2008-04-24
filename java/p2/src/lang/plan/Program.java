package lang.plan;

import java.util.ArrayList;
import java.util.List;

import types.basic.Tuple;
import types.exception.UpdateException;
import types.table.Key;
import types.table.ObjectTable;
import types.table.Schema;

public class Program {
	
	public static class ProgramTable extends ObjectTable {
		private static final Integer[] PRIMARY_KEY = {0};
		
		private static final Schema schema = 
			new Schema(new Schema.Entry("Name",    String.class),
					   new Schema.Entry("Owner",   String.class),
					   new Schema.Entry("Program", Program.class));

		public ProgramTable(String name, Schema schema, Key key) {
			super(ProgramTable.class.getName(), schema, key);
			// TODO Auto-generated constructor stub
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
	
	private List<Event> events;
	
	private List<Watch> watches;
	
	private List<Fact> facts;
	
	private List<Rule> rules;
	
	public Program(String name) {
		this.name    = name;
		this.tables  = new ArrayList<Table>();
		this.events  = new ArrayList<Event>();
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
		for (Event e : events) {
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
	
	public void table(Table t) {
		tables.add(t);
	}
	
	public void event(Event e) {
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
}
