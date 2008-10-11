package jol.types.operator;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;
import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.exception.P2RuntimeException;
import jol.types.function.TupleFunction;
import jol.core.Runtime;

/**
 * The interface to all join operators.
 */
public abstract class Join extends Operator {
	
	/**
	 * A table function used to extract values from tuples 
	 * taken from a specific {@link jol.types.table.Table} object.
	 *
	 */
	private class TableField implements TupleFunction<Comparable> {
		/** The field type. */
		private Class type;
		/** The field position within the tuple. */
		private Integer position;
		/**
		 * Create a new TableField.
		 * @param type The field type.
		 * @param position The field position.
		 */
		public TableField(Class type, Integer position) { 
			this.type = type;
			this.position=position; 
		}
		/** Extracts the value from the tuple field position. */
		public Comparable evaluate(Tuple tuple) throws P2RuntimeException {
			return tuple.value(this.position);
		}
		/** The type of value returned by {@link #evaluate(Tuple)}. */
		public Class returnType() {
			return this.type;
		}
	}
	
	/**
	 * Evaluates a single join attribute.
	 */
	private class JoinFilter {
		/** Left hand side value accessor. */
		private TupleFunction<Comparable> lhs;
		
		/** Right hand side value accessor. */
		private TupleFunction<Comparable> rhs;

		/**
		 * Create a new join filter. 
		 * @param lhs The left hand side accessor.
		 * @param rhs The right hand side accessor.
		 */
		private JoinFilter(TupleFunction<Comparable> lhs, 
				           TupleFunction<Comparable> rhs) {
			this.lhs = lhs;
			this.rhs = rhs;
		}
		
		/**
		 * Evaluate a tuple from the outer relation and a tuple 
		 * from the inner relation along a single join attribute.
		 * @param outer Tuple from the outer.
		 * @param inner Tuple from the inner.
		 * @return true if join succeeds, false otherwise.
		 * @throws P2RuntimeException
		 */
		public Boolean evaluate(Tuple outer, Tuple inner) throws P2RuntimeException {
			Comparable lvalue = null;
			Comparable rvalue = null;
			if (this.lhs instanceof TableField) {
				lvalue = this.lhs.evaluate(inner); 
			}
			else {
				lvalue = this.lhs.evaluate(outer); 
			}
			
			if (this.rhs instanceof TableField) {
				rvalue = this.rhs.evaluate(inner);
			}
			else {
				rvalue = this.rhs.evaluate(outer);
			}
			return lvalue.compareTo(rvalue) == 0;
		}
	}
	
	/** The predicate of the table being joined with the input tuples. */
	protected Predicate predicate;
	
	/** The output schema. */
	protected Schema schema;
	
	/** A list of join filters, one for each common join attribute. */
	private List<JoinFilter> filters;
	
	/**
	 * Create a new join operator.
	 * @param context The runtime context.
	 * @param predicate The predicate representing the table being joined.
	 * @param input The schema of the input tuples that are to be joined with the (inner) table.
	 */
	public Join(Runtime context, Predicate predicate, Schema input) {
		super(context, predicate.program(), predicate.rule());
		this.predicate = predicate;
		this.filters = filters(predicate);
		this.schema = input.join(predicate.schema());
		
	}
	
	@Override
	public Schema schema() {
		return this.schema;
	}

	@Override
	public Set<Variable> requires() {
		return predicate.requires();
	}
	
	/**
	 * Apply all join filters.
	 * @param outer Tuple from the outer relation.
	 * @param inner Tuple from the inner relation.
	 * @return true if all join filters succeed, false otherwise.
	 * @throws P2RuntimeException
	 */
	protected Boolean validate(Tuple outer, Tuple inner) throws P2RuntimeException {
		for (JoinFilter filter : filters) {
			if (filter.evaluate(outer, inner) == Boolean.FALSE) {
				return false;
			}
		}
		return true;
	}
	
	/**
	 * Create a list of join filters based on the predicate schema
	 * and the input tuple schema. A filter will be created for each
	 * variable that matches between these two schemas.
	 * @param predicate The predicate that references the inner table.
	 * @return A list of join filters.
	 */
	private List<JoinFilter> filters(Predicate predicate) {
		List<JoinFilter> filters = new ArrayList<JoinFilter>();
		
		Hashtable<String, Variable> positions = new Hashtable<String, Variable>();
		for (jol.lang.plan.Expression arg : predicate) {
			assert(arg.position() >= 0);
			
			if (arg instanceof Variable) {
				Variable var = (Variable) arg;
				if (positions.containsKey(var.name())) {
					Variable prev = positions.get(var.name());
					filters.add(new JoinFilter(
								   new TableField(prev.type(), prev.position()), 
								   new TableField(var.type(), prev.position())));
				}
				else {
					positions.put(var.name(), var);
				}
			}
			else {
				filters.add(new JoinFilter(
						    new TableField(arg.type(), 
						    		       arg.position()), 
						    		       arg.function()));
			}
		}
		
		return filters;
	}
}
