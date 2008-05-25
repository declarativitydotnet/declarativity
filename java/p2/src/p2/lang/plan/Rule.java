package p2.lang.plan;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import p2.exec.BasicQuery;
import p2.exec.Query;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.PlannerException;
import p2.types.exception.UpdateException;
import p2.types.operator.EventFilter;
import p2.types.operator.Operator;
import p2.types.operator.Projection;
import p2.types.table.AggregateImpl;
import p2.types.table.HashIndex;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;

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
		protected boolean delete(Tuple tuple) throws UpdateException {
			return super.delete(tuple);
		}
	}
	
	private String program;
	
	private String name;
	
	private java.lang.Boolean deletion;
	
	private Predicate head;
	
	private List<Term> body;
	
	private boolean aggregation;
	
	public Rule(xtc.tree.Location location, String name, java.lang.Boolean deletion, Predicate head, List<Term> body) {
		super(location);
		this.name = name;
		this.deletion = deletion;
		this.head = head;
		this.body = body;
		this.aggregation = false;
		for (Expression arg : head) {
			if (arg instanceof Aggregate) {
				/* assertion: only 1 aggregate. */
				assert(this.aggregation == false);
				this.aggregation = true;
			}
		}
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
	
	public int compareTo(Clause o) {
		if (o instanceof Rule) {
			Rule other = (Rule) o;
			String otherName = other.program + ":" + other.name;
			String myName    = this.program + ":" + this.name;
			return otherName.compareTo(myName);
		}
		return -1;
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
		
		Program.rule.force(new Tuple(Program.rule.name(), program, name, deletion, this));
	}

	public List<Query> query() throws PlannerException {
		/* First search for an event predicate. */
		Predicate event   = null;
		Function function = null;
		for (Term term : body) {
			if (term instanceof Predicate) {
				Predicate pred = (Predicate) term;
				Table table = Table.table(pred.name());
				if (table.type() == Table.Type.EVENT ||
				    pred.event() != Table.Event.NONE) {
					if (event != null) {
						throw new PlannerException("Multiple event predicates in rule " + name);
					}
					/* Plan a query with this event predicate as input. */
					event = pred;
				}
			}
			else if (term instanceof Function) {
				function = (Function) term;
				event = function.predicate();
				
			}
		}
		
		List<Query> queries = new ArrayList<Query>();
		if (event != null) {
			List<Operator> operators = new ArrayList<Operator>();
			
			EventFilter efilter = new EventFilter(event);
			if (efilter.filters() > 0) {
				operators.add(efilter);
			}
			
			/* Table function go first to immediately evaluate 
			 * input event tuples. */
			if (function != null) {
				operators.add(function.operator());
			}
			
			/* Add all remaining terms in the rule body. */
			for (Term term : body) {
				if (!term.equals(event)) {
					operators.add(term.operator());
				}
			}
			
			queries.add(new BasicQuery(program, name, deletion, event, new Projection(this.head), operators));
		}
		else {
			/* Perform delta rewrite. */
			Set<String> eventPredicates = new HashSet<String>();
			for (Term term1 : body) {
				if (!(term1 instanceof Predicate)) {
					continue;
				}
				else {
					Predicate pred = (Predicate) term1;
					if (pred.notin() || eventPredicates.contains(pred.name())) {
						continue;
					}
					eventPredicates.add(pred.name());
				}
				
				Predicate delta = (Predicate) term1;
				List<Operator> operators = new ArrayList<Operator>();
				EventFilter efilter = new EventFilter(delta);
				if (efilter.filters() > 0) {
					operators.add(efilter);
				}
				
				for (Term term2 : body) {
					if (!term2.equals(delta)) {
						operators.add(term2.operator());
					}
				}
				
				queries.add(new BasicQuery(program, name, deletion, delta, 
							               new Projection(this.head), operators));
			}
			
		}
		return queries;
	}
	
}
