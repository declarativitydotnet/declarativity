package lang.ast;

import java.util.ArrayList;
import java.util.List;

public class Program {
	
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
