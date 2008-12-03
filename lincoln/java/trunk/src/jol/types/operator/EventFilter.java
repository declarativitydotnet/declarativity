package jol.types.operator;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;
import jol.core.Runtime;

/**
 * Event predicates can contain constant values in the list of arguments.
 * These constants represent *filters* that are applied to the input
 * tuple stream. Any tuples that satisfy these filters are sent to the
 * output.
 */
public class EventFilter extends Operator {
	/**
	 * A filter extracts a field from the input tuple and compares it
	 * to some constant value. If this comparison succeeds then the 
	 * filter succeeds. 
	 */
	private class ConstantFilter implements TupleFunction<Boolean> {
		/** The value position within the tuple used in the comparison. */
		private Integer position;
		
		/** The function that extracts the constant used in the comparison. */
		private TupleFunction<Comparable> function;

		/**
		 * Create a new filter.
		 * @param position The value position.
		 * @param function The constant function extractor.
		 */
		private ConstantFilter(Integer position, TupleFunction<Comparable> function) {
			this.position = position;
			this.function = function;
		}

		/** @return true if filter is satisfied, false otherwise.  */
		public Boolean evaluate(Tuple tuple) throws JolRuntimeException {
			Comparable fvalue = function.evaluate(tuple);
			Comparable tvalue = tuple.value(position);
			
			return (fvalue == tvalue || fvalue.compareTo(tvalue) == 0);
		}

		/** The constant type. */
		public Class returnType() {
			return function.returnType();
		}
		
	}
	
	/**
	 * A filter that ensures two attributes values are the same.
	 */
	private class AttributeFilter implements TupleFunction<Boolean> {
		private Class type;
		private int a1;
		private int a2;

		/**
		 * Create a new filter.
		 * @param position The value position.
		 * @param function The constant function extractor.
		 */
		private AttributeFilter(Class type, int a1, int a2) {
			this.type = type;
			this.a1 = a1;
			this.a2 = a2;
		}

		/** @return true if filter is satisfied, false otherwise.  */
		public Boolean evaluate(Tuple tuple) throws JolRuntimeException {
			Comparable a1 = tuple.value(this.a1);
			Comparable a2 = tuple.value(this.a2);
			return (a1 == a2 || 
					(a1 != null && a2 != null && a1.compareTo(a2) == 0));
		}

		/** The constant type. */
		public Class returnType() {
			return type;
		}
	}
	
	/** The event predicate. */
	private Predicate predicate;
	
	/** A list of filters, one for each constant in the event predicate. */
	private List<TupleFunction<Boolean>> filters;
	

	/**
	 * Create a new event filter operator.
	 * @param context The runtime context.
	 * @param predicate The event predicate. 
	 */
	public EventFilter(Runtime context, Predicate predicate) {
		super(context, predicate.program(), predicate.rule());
		this.predicate = predicate;
		this.filters = new ArrayList<TupleFunction<Boolean>>();
		
		HashMap<String, Variable> variables = new HashMap<String, Variable>();
		for (jol.lang.plan.Expression arg : predicate) {
			if (arg instanceof Variable) {
				Variable var = (Variable) arg;
				if (variables.containsKey(var.name())) {
					Variable previous = variables.get(var.name());
					this.filters.add(
							new AttributeFilter(previous.type(),
							                    previous.position(), 
							                    var.position()));
				}
				else {
					variables.put(var.name(), var);
				}
			}
			else {
				this.filters.add(new ConstantFilter(arg.position(), arg.function()));
			}
		}
	}
	
	/**
	 * Create a new event filter.
	 * @param context The runtime context.
	 * @param predicate The event predicate.
	 * @param filter A filter to apply to the input tuples that determines filter success.
	 */
	public EventFilter(Runtime context, Predicate predicate, TupleFunction<Boolean> filter) {
		super(context, predicate.program(), predicate.rule());
		this.predicate = predicate;
		this.filters = new ArrayList<TupleFunction<Boolean>>();
		this.filters.add(filter);
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		if (filters.size() == 0)
			return tuples;

		TupleSet result = new TupleSet(tuples.name());

		for (Tuple tuple : tuples) {
		    boolean valid = true;
		    for (TupleFunction<Boolean> filter : this.filters) {
		        valid = filter.evaluate(tuple);
		        if (!valid) break;
		    }
		    if (valid) {
		    	result.add(tuple);
		    }
		}
		
		return result;
	}

	@Override
	public Set<Variable> requires() {
		return this.predicate.requires();
	}

	@Override
	public Schema schema() {
		return this.predicate.schema().clone();
	}

	@Override
	public String toString() {
		return "EVENT FILTER " + predicate + ": FILTERS " + this.filters;
	}
	
	/**
	 * The number of constant filters.
	 * @return The number of constant filters.
	 */
	public int filters() {
		return this.filters.size();
	}

}
