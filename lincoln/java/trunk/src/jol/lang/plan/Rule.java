package jol.lang.plan;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import jol.core.Runtime;
import jol.exec.BasicQuery;
import jol.exec.Query;
import jol.lang.plan.Watch.WatchTable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.exception.PlannerException;
import jol.types.exception.UpdateException;
import jol.types.operator.EventFilter;
import jol.types.operator.Operator;
import jol.types.operator.Projection;
import jol.types.operator.RemoteBuffer;
import jol.types.table.EventTable;
import jol.types.table.HashIndex;
import jol.types.table.Index;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.Table;
import jol.types.table.TableName;

public class Rule extends Clause {
	
	public static class RuleTable extends ObjectTable {
		public static final TableName TABLENAME = new TableName(GLOBALSCOPE, "rule");
		public static final Key PRIMARY_KEY = new Key(0,1);
		
		public enum Field {PROGRAM, RULENAME, PUBLIC, ASYNC, DELETE, OBJECT};
		public static final Class[] SCHEMA =  {
			String.class,             // Program name
			String.class,             // Rule name
			java.lang.Boolean.class,  // public rule?
			java.lang.Boolean.class,  // async rule?
			java.lang.Boolean.class,  // delete rule?
			Rule.class                // Rule object
		};

		public RuleTable(Runtime context) {
			super(context, new TableName(GLOBALSCOPE, "rule"), PRIMARY_KEY, new TypeList(SCHEMA));
			Key programKey = new Key(Field.PROGRAM.ordinal());
			Index index = new HashIndex(context, this, programKey, Index.Type.SECONDARY);
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
			object.isAsync   = (java.lang.Boolean) tuple.value(Field.ASYNC.ordinal());
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
	
	private java.lang.Boolean isAsync;
	
	private Predicate head;
	
	private List<Term> body;
	
	private boolean aggregation;
	
	
	public Rule(xtc.tree.Location location, String name, 
			    java.lang.Boolean isPublic, java.lang.Boolean isAsync,
			    java.lang.Boolean isDelete, 
			    Predicate head, List<Term> body) {
		super(location);
		this.name = name;
		this.isPublic = isPublic;
		this.isAsync  = isAsync;
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
		String value = (isPublic ? "public " : "") + 
					   (isAsync ? "async " : "") + name + 
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
	
	@Override
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
	public void set(Runtime context, String program) throws UpdateException {
		this.head.set(context, program, this.name, 0);
		for (int i = 0; i < this.body.size(); i++) {
			this.body.get(i).set(context, program, this.name, i+1);
		}
		
		context.catalog().table(RuleTable.TABLENAME).force(new Tuple(program, name, isPublic, isAsync, isDelete, this));
	}

	public List<Query> query(Runtime context) throws PlannerException {
		/* First search for an event predicate. */
		Predicate event   = null;
		
		List<Selection> selections = new ArrayList<Selection>();
		for (Term term : body) {
			if (term instanceof Predicate) {
				Predicate pred = (Predicate) term;
				selections.addAll(canonicalize(pred));

				Table table = context.catalog().table(pred.name());
				if (table.type() == Table.Type.EVENT ||
				    pred.event() != Predicate.Event.NONE) {
					if (event != null) {
						throw new PlannerException("Multiple event predicates in rule " + name + 
								                   " location " + pred.location());
					}
					/* Plan a query with this event predicate as input. */
					event = pred;
				}
			
				if (event instanceof Function) {
					event.event(Predicate.Event.INSERT);
				}
			}
		}
		body.addAll(selections);

		if (event != null) {
			return query(context, head, event, body);
		}
		else {
			return mviewQuery(context, head, body);
		}
	}
	
	private List<Selection> canonicalize(Predicate pred) {
		List<Selection> selections        = new ArrayList<Selection>();
		List<Expression> canonicalization = new ArrayList<Expression>();
		for (Expression argument : pred.arguments()) {
			if (!(argument instanceof Variable) &&
					argument.variables().size() > 0) {
				Variable dontcare = new DontCare(argument.type());
				selections.add(new Selection(new Boolean(Boolean.EQUAL, 
						                     dontcare.clone(), argument)));
				canonicalization.add(dontcare);
			}
			else {
				canonicalization.add(argument);
			}
		}
		pred.arguments(canonicalization);
		return selections;
	}
	
	private List<Query> mviewQuery(Runtime context, Predicate head, List<Term> body) throws PlannerException {
		List<Query> queries = new ArrayList<Query>();
		/* Perform delta rewrite. */
		for (Term term1 : body) {
			if (!(term1 instanceof Predicate) || ((Predicate)term1).notin()) {
				continue;
			}
			
			Predicate delta = (Predicate) term1;
			queries.addAll(query(context, head, delta, body));
		}
		return queries;
	}
	
	private List<Query> localize(Runtime context, Predicate head, Predicate event, List<Term> body) throws PlannerException {
		List<Query> queries = new ArrayList<Query>();
		
		/* Group all predicate terms by the location variable. */
		Map<String, List<Term>> groupByLocation = new HashMap<String, List<Term>>();
		List<Term> remainder = new ArrayList<Term>(); // Remaining terms.
		for (Term t : body) {
			if (t instanceof Predicate) {
				Predicate p = (Predicate) t;

				if (p.locationVariable() == null)
					throw new PlannerException("Predicate \"" + p.name() + "\" in " +
											   "localized rule \"" + head.name() +
											   "\" has no location specifier!");

				String loc = p.locationVariable().name();
				if (!groupByLocation.containsKey(loc)) {
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
		 * rule group. The original event will be the first group. Subsequent 
		 * groups will be triggered off the intermediate event predicates
		 * created during the localization process. The final group will 
		 * project onto the original head predicate. */
		while (groupByLocation.size() > 0) {
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
				context.catalog().register(intermediateEvent);
				Predicate intermediate = new Predicate(context, false, intermediateEvent.name(), Predicate.Event.NONE, intermediateSchema);
				intermediate.program  = event.program();
				intermediate.rule     = event.rule();
				intermediate.position = 0;
				
				/* Create a query with the intermediate as the head predicate. */
				queries.addAll(query(context, intermediate, event, intermediateBody));
				
				/* the intermediate predicate will be the event predicate in the next (localized) rule. */
				event = intermediate;
			}
			else {
				/* This is the final group that projects back to the original head. */
				intermediateBody.addAll(remainder); // Tack on any remainder terms (e.g., selections, assignments).
				queries.addAll(query(context, head, event, intermediateBody));
			}
		}
		return queries;
	}

	
	private List<Query> query(Runtime context, Predicate head, Predicate event, List<Term> body) throws PlannerException {
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
					return localize(context, head, event, body);
				}
			}
		}
		
		
		EventFilter efilter = new EventFilter(context, event);
		if (efilter.filters() > 0) {
			operators.add(efilter);
		}
		
		WatchTable watch = (WatchTable) context.catalog().table(WatchTable.TABLENAME);
		if (watch.watched(program, event.name(), jol.types.operator.Watch.Modifier.RECEIVE) != null) {
			operators.add(
					new jol.types.operator.Watch(context, program, name, event.name(), 
							                    jol.types.operator.Watch.Modifier.RECEIVE));
		}
		
		if (event instanceof Function) {
			operators.add(event.operator(context, event.schema().clone()));
		}
		
		Schema schema = event.schema().clone();
		List<Assignment> assignments = new ArrayList<Assignment>();
		for (Term term : body) {
			if (term instanceof Assignment) assignments.add((Assignment)term);
			else if (!term.equals(event)) {
				Operator oper = term.operator(context, schema);
				operators.add(oper);
				schema = oper.schema();
			}
		}
		
		for (Assignment assignment : assignments) {
			Operator oper = assignment.operator(context, schema);
			operators.add(oper);
			schema = oper.schema();
		}
		
		operators.add(new Projection(context, head, schema));
		
		if (watch.watched(program, head.name(), jol.types.operator.Watch.Modifier.SEND) != null) {
			operators.add(
					new jol.types.operator.Watch(context, program, name, head.name(), 
							                    jol.types.operator.Watch.Modifier.SEND));
		}
		
		Variable headLoc  = head.locationVariable();
		Variable eventLoc = event.locationVariable();
		if (headLoc != null && eventLoc != null && !headLoc.equals(eventLoc)) {
			/* Plan remote buffer operator. */
			operators.add(new RemoteBuffer(context, head, isDelete));
		}
		
		query.add(new BasicQuery(context, program, name, isPublic, isAsync, isDelete, event, head, operators));
		return query;
	}

}
