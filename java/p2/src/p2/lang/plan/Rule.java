package p2.lang.plan;

import java.util.List;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;
import p2.types.table.Key;
import p2.types.table.ObjectTable;

public class Rule extends Clause {
	
	public static class RuleTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public enum Field {PROGRAM, RULENAME, OBJECT};
		public static final Class[] SCHEMA =  {
			String.class,    // Program name
			String.class,    // Rule name
			Rule.class      // Rule object
		};

		public RuleTable() {
			super("rule", PRIMARY_KEY, new TypeList(SCHEMA));
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
	
	private java.lang.Boolean deletion;
	
	private Predicate head;
	
	private List<Term> body;
	
	public Rule(String name, java.lang.Boolean deletion, Predicate head, List<Term> body) {
		this.name = name;
		this.deletion = deletion;
		this.head = head;
		this.body = body;
	}
	
	@Override
	public String toString() {
		String value = name + (deletion ? " delete " : " ") + head + " :- \n";
		for (int i = 0; i < body.size(); i++) {
			value += "\t" + body.get(i);
			if (i + 1 < body.size()) {
				value += ",\n";
			}
			else {
				value += ";\n";
			}
		}
		return value;
	}
}
