package p2.lang.plan;

import java.util.ArrayList;
import java.util.List;

import p2.exec.BasicQuery;
import p2.exec.Query;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TypeList;
import p2.types.exception.PlannerException;
import p2.types.exception.UpdateException;
import p2.types.operator.Operator;
import p2.types.operator.Projection;
import p2.types.table.HashIndex;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.ObjectTable;

public class Rule extends Clause {
	
	public static class RuleTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public enum Field {PROGRAM, RULENAME, DELETION, OBJECT};
		public static final Class[] SCHEMA =  {
			String.class,   // Program name
			String.class,   // Rule name
			Boolean.class,  // deletion rule?
			Rule.class      // Rule object
		};

		public RuleTable() {
			super("rule", PRIMARY_KEY, new TypeList(SCHEMA));
			Key programKey = new Key(Field.PROGRAM.ordinal());
			Index index = new HashIndex(this, programKey, Index.Type.SECONDARY);
			this.secondary.put(programKey, index);
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
	
	private String program;
	
	private String name;
	
	private java.lang.Boolean deletion;
	
	private Predicate head;
	
	private List<Term> body;
	
	public Rule(xtc.tree.Location location, String name, java.lang.Boolean deletion, Predicate head, List<Term> body) {
		super(location);
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
	
	public Predicate head() {
		return this.head;
	}
	
	public List<Term> body() {
		return this.body;
	}
	
	@Override
	public void set(String program) throws UpdateException {
		this.head.set(program, this.name, 0);
		for (int i = 0; i < this.body.size(); i++) {
			this.body.get(i).set(program, this.name, i+1);
		}
		
		Tuple me = new Tuple(Program.rule.name(), program, name, deletion, this);
		Program.rule.force(me);
	}

	public List<Query> query() throws PlannerException {
		/* First search for an event predicate. */
		Predicate event = null;
		for (Term term : body) {
			if (term instanceof Predicate) {
				Predicate pred = (Predicate) term;
				if (pred.event() != Predicate.EventModifier.NONE) {
					if (event != null) {
						throw new PlannerException("Multiple event predicates in rule " + name);
					}
					/* Plan a query with this event predicate as input. */
					event = pred;
				}
			}
		}
		
		List<Query> queries = new ArrayList<Query>();
		if (event != null) {
			List<Operator> operators = new ArrayList<Operator>();
			for (Term term : body) {
				if (!term.equals(event)) {
					operators.add(term.operator());
				}
			}
			queries.add(new BasicQuery(program, name, deletion, event, new Projection(this.head), operators));
		}
		else {
			/* Perform delta rewrite. */
			for (Term term1 : body) {
				if (!(term1 instanceof Predicate)) {
					continue;
				}
				else {
					Predicate pred = (Predicate) term1;
					if (pred.notin()) continue;
				}
				
				Predicate delta = (Predicate) term1;
				List<Operator> operators = new ArrayList<Operator>();
				for (Term term2 : body) {
					if (!term2.equals(delta)) {
						operators.add(term2.operator());
					}
				}
				queries.add(new BasicQuery(program, name, deletion, delta, new Projection(this.head), operators));
			}
			
		}
		return queries;
	}
	
}
