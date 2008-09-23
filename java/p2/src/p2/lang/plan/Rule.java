package p2.lang.plan;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;

import p2.exec.BasicQuery;
import p2.exec.Query;
import p2.lang.plan.Predicate.Field;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.PlannerException;
import p2.types.exception.UpdateException;
import p2.types.function.TupleFunction;
import p2.types.operator.EventFilter;
import p2.types.operator.Operator;
import p2.types.operator.Projection;
import p2.types.operator.RemoteBuffer;
import p2.types.table.Aggregation;
import p2.types.table.EventTable;
import p2.types.table.HashIndex;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.ObjectTable;
import p2.types.table.Table;
import p2.types.table.TableName;
import p2.core.Periodic;
import p2.lang.Compiler;

public class Rule extends Clause {
	
	public static class RuleTable extends ObjectTable {
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public enum Field {PROGRAM, RULENAME, PUBLIC, DELETE, OBJECT};
		public static final Class[] SCHEMA =  {
			String.class,             // Program name
			String.class,             // Rule name
			java.lang.Boolean.class,  // public rule?
			java.lang.Boolean.class,  // delete rule?
			Rule.class                // Rule object
		};

		public RuleTable() {
			super(new TableName(GLOBALSCOPE, "rule"), PRIMARY_KEY, new TypeList(SCHEMA));
			Key programKey = new Key(Field.PROGRAM.ordinal());
			Index index = new HashIndex(this, programKey, Index.Type.SECONDARY);
			this.secondary.put(programKey, index);
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Rule object = (Rule) tuple.value(Field.OBJECT.ordinal());
			if (object == null) {
				throw new UpdateException("Predicate object null");
			}
			object.program   = (String) tuple.value(Field.PROGRAM.ordinal());
			object.name      = (String) tuple.value(Field.RULENAME.ordinal());
			object.isPublic  = (java.lang.Boolean) tuple.value(Field.PUBLIC.ordinal());
			object.isDelete  = (java.lang.Boolean) tuple.value(Field.DELETE.ordinal());
			return super.insert(tuple);
		}
		
		@Override
		protected boolean delete(Tuple tuple) throws UpdateException {
			return super.delete(tuple);
		}
	}
	
	private String program;
	
	private String name;
	
	private java.lang.Boolean isDelete;
	
	private java.lang.Boolean isPublic;
	
	private Predicate head;
	
	private List<Term> body;
	
	private boolean aggregation;
	
	
	public Rule(xtc.tree.Location location, String name, 
			    java.lang.Boolean isPublic, java.lang.Boolean isDelete, 
			    Predicate head, List<Term> body) {
		super(location);
		this.name = name;
		this.isPublic = isPublic;
		this.isDelete = isDelete;
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
		String value = (isPublic ? "public " : "") + name + 
		               (isDelete ? " delete " : " ") + head + " :- \n";
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
		
		Compiler.rule.force(new Tuple(program, name, isPublic, isDelete, this));
	}

	public List<Query> query(TupleSet periodics) throws PlannerException {
		/* First search for an event predicate. */
		Predicate event   = null;
		Variable locVariable = null;
		
		for (Term term : body) {
			if (term instanceof Predicate) {
				Predicate pred = (Predicate) term;

				Table table = Table.table(pred.name());
				if (table.type() == Table.Type.EVENT ||
				    pred.event() != Table.Event.NONE) {
					if (event != null) {
						throw new PlannerException("Multiple event predicates in rule " + name + 
								                   " location " + pred.location());
					}
					/* Plan a query with this event predicate as input. */
					event = pred;
				}
			
				if (event instanceof Function) {
					event.event(Table.Event.INSERT);
				}
			}
		}
		
		List<Query> queries = new ArrayList<Query>();
		if (event != null) {
			return query(periodics, head, event, body);
		}
		else {
			return mviewQuery(head, body);
		}
	}
	
	private List<Query> mviewQuery(Predicate head, List<Term> body) throws PlannerException {
		List<Query> queries = new ArrayList<Query>();
		/* Perform delta rewrite. */
		for (Term term1 : body) {
			if (!(term1 instanceof Predicate) || ((Predicate)term1).notin()) {
				continue;
			}
			
			Predicate delta = (Predicate) term1;
			queries.addAll(query(null, head, delta, body));
		}
		return queries;
	}
	
	private List<Query> localize(TupleSet periodics, Predicate head, Predicate event, List<Term> body) throws PlannerException {
		List<Query> queries = new ArrayList<Query>();
		
		/* Group all predicate terms by the location variable. */
		Hashtable<String, List<Term>> groupByLocation = new Hashtable<String, List<Term>>();
		List<Term> remainder = new ArrayList<Term>(); // Remaining terms.
		for (Term t : body) {
			if (t instanceof Predicate) {
				Predicate p = (Predicate) t;
				String loc = p.locationVariable().name();
				if (!groupByLocation.contains(loc)) {
					groupByLocation.put(loc, new ArrayList<Term>());
				}
				groupByLocation.get(loc).add(p);
			}
			else {
				remainder.add(t);
			}
		}
		
		/* Create the set of localized rules. */
		/* Grab the location variable representing from the event for this 
		 * rule group. The orginal event will be the first group. Subsequent 
		 * groups will be triggered off the intermediate event predicates
		 * created during the localization process. The final group will 
		 * project onto the orginal head predicate. */
		while(groupByLocation.size() > 0) {
			String location           = event.locationVariable().name();
			Schema intermediateSchema = event.schema();
			String intermediateName   = new String(this.name + "_intermediate_" + event.name().name);
		
			/* Get the set of predicates in this location group. */
			List<Term> intermediateBody = groupByLocation.get(location);
			groupByLocation.remove(location);

			if (groupByLocation.size() > 0) {
				for (Term t : intermediateBody) {
					Predicate p = (Predicate) t;
					intermediateSchema = intermediateSchema.join(p.schema());
				}
				
				/* Turn off location variable(s) in intermediate schema. */
				for (Variable v : intermediateSchema.variables()) {
					v.loc(false);
				}
				
				/* Locate the next location variable from remaining groups. 
				 * The location variable must appear in the schema of the intermediate
				 * schema (That is we need to know its value). */
				for (String loc : groupByLocation.keySet()) {
					Variable var = intermediateSchema.variable(loc);
					if (var != null) {
						var.loc(true); // Make this the location variable.
						break;
					}
				}
			
				EventTable
				intermediateEvent = new EventTable(new TableName(this.program, intermediateName), 
					                           new TypeList(intermediateSchema.types()));
				Predicate intermediate = new Predicate(false, intermediateEvent.name(), Table.Event.NONE, intermediateSchema);
				intermediate.program  = event.program();
				intermediate.rule     = event.rule();
				intermediate.position = 0;
				
				/* Create a query with the intermediate as the head predicate. */
				queries.addAll(query(periodics, intermediate, event, intermediateBody));
				
				/* the intermediate predicate will be the event predicate in the next (localized) rule. */
				event = intermediate;
			}
			else {
				/* This is the final group that projects back to the orginal head. */
				intermediateBody.addAll(remainder); // Tack on any remainder terms (e.g., selections, assignments).
				queries.addAll(query(periodics, head, event, intermediateBody));
			}
		}
		return queries;
	}

	
	private List<Query> query(TupleSet periodics, Predicate head, Predicate event, List<Term> body) throws PlannerException {
		List<Query>    query     = new ArrayList<Query>();
		List<Operator> operators = new ArrayList<Operator>();
		
		Variable loc = event.locationVariable();
		for (Term t : body) {
			if (t instanceof Predicate) {
				Predicate p = (Predicate) t;
				if (loc == null) {
					if (p.locationVariable() != null) {
						throw new PlannerException("Can't mix location variables in a local rule!");
					}
				}
				else if (!loc.equals(p.locationVariable())) {
					return localize(periodics, head, event, body);
				}
			}
		}
		
		
		if (event.name().name.equals("periodic") && 
				! event.name().scope.equals(Table.GLOBALSCOPE)) {
			Long period = (Long) ((Value) event.argument(Periodic.Field.PERIOD.ordinal())).value();
			Long ttl    = (Long) ((Value) event.argument(Periodic.Field.TTL.ordinal())).value();
			Long count  = (Long) ((Value) event.argument(Periodic.Field.COUNT.ordinal())).value();
			List<Comparable> values = new ArrayList<Comparable>();
			values.add(event.identifier());
			for (int i = 1; i < event.arguments(); i++) {
				values.add(((Value<Comparable>) event.argument(i)).value());
			}
			periodics.add(new Tuple(values));
			
			final String identifier = event.identifier();
			TupleFunction<java.lang.Boolean> periodicFilter = new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple)
						throws P2RuntimeException {
					return identifier.equals((String)tuple.value(Periodic.Field.IDENTIFIER.ordinal()));
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
			EventFilter efilter = new EventFilter(event, periodicFilter);
			operators.add(efilter);
		}
		else {
			EventFilter efilter = new EventFilter(event);
			if (efilter.filters() > 0) {
				operators.add(efilter);
			}
		}
		
		if (Compiler.watch.watched(program, event.name(), p2.types.operator.Watch.Modifier.RECEIVE) != null) {
			operators.add(
					new p2.types.operator.Watch(program, name, event.name(), 
							                    p2.types.operator.Watch.Modifier.RECEIVE));
		}
		
		if (event instanceof Function) {
			operators.add(event.operator(event.schema().clone()));
		}
		
		Schema schema = event.schema().clone();
		for (Term term : body) {
			if (!term.equals(event)) {
				Operator oper = term.operator(schema);
				operators.add(oper);
				schema = oper.schema();
			}
		}
		
		operators.add(new Projection(head));
		
		if (Compiler.watch.watched(program, head.name(), p2.types.operator.Watch.Modifier.SEND) != null) {
			operators.add(
					new p2.types.operator.Watch(program, name, head.name(), 
							                    p2.types.operator.Watch.Modifier.SEND));
		}
		
		Variable headLoc  = head.locationVariable();
		Variable eventLoc = event.locationVariable();
		if (headLoc != null && eventLoc != null && !headLoc.equals(eventLoc)) {
			/* Plan remote buffer operator. */
			operators.add(new RemoteBuffer(head, isDelete));
		}
		
		query.add(new BasicQuery(program, name, isPublic, isDelete, event, head, operators));
		return query;
	}

}
