package jol.types.operator;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import jol.core.Runtime;
import jol.lang.plan.Expression;
import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.Filter;
import jol.types.function.TupleFunction;

/**
 * The interface to all join operators.
 */
public abstract class Join extends Operator {

	/**
	 * A table function used to extract values from tuples
	 * taken from a specific {@link jol.types.table.Table} object.
	 *
	 */
	private class TableField implements TupleFunction<Object> {
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
			this.position = position;
		}
		/** Extracts the value from the tuple field position. */
		public Object evaluate(Tuple tuple) throws JolRuntimeException {
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
		private TupleFunction<Object> lhs;

		/** Right hand side value accessor. */
		private TupleFunction<Object> rhs;

		/**
		 * Create a new join filter.
		 * @param lhs The left hand side accessor.
		 * @param rhs The right hand side accessor.
		 */
		private JoinFilter(TupleFunction<Object> lhs,
				           TupleFunction<Object> rhs) {
			this.lhs = lhs;
			this.rhs = rhs;
		}

		/**
		 * Evaluate a tuple from the outer relation and a tuple
		 * from the inner relation along a single join attribute.
		 * @param outer Tuple from the outer.
		 * @param inner Tuple from the inner.
		 * @return true if join succeeds, false otherwise.
		 * @throws JolRuntimeException
		 */
		public Boolean evaluate(Tuple outer, Tuple inner) throws JolRuntimeException {
			Object lvalue = this.lhs.evaluate(outer);
			Object rvalue = this.rhs.evaluate(inner);
			return lvalue.equals(rvalue);
		}
	}

	/** A list of join filters, one for each common join attribute. */
	private List<JoinFilter> joinFilters;

	private List<Filter> predicateFilters;

	private List<Integer> innerNonJoinPositions;

	/**
	 * Create a new join operator.
	 * @param context The runtime context.
	 * @param predicate The predicate representing the table being joined.
	 * @param input The schema of the input tuples that are to be joined with the (inner) table.
	 * @throws PlannerException
	 */
	public Join(Runtime context, Predicate predicate, Schema input) throws PlannerException {
		super(context, predicate.program(), predicate.rule());
		Schema innerSchema = predicate.schema();
		this.innerNonJoinPositions = new ArrayList<Integer>();
		for (Variable var : innerSchema.variables()) {
			if (!input.contains(var)) {
				this.innerNonJoinPositions.add(innerSchema.position(var.name()));
			}
		}
		initFilters(predicate, input);
	}

	/**
	 * Apply all join filters.
	 * @param outer Tuple from the outer relation.
	 * @param inner Tuple from the inner relation.
	 * @return true if all join filters succeed, false otherwise.
	 * @throws JolRuntimeException
	 */
	private Boolean validate(Tuple outer, Tuple inner) throws JolRuntimeException {
		for (JoinFilter filter : this.joinFilters) {
			if (filter.evaluate(outer, inner) == Boolean.FALSE) {
				return false;
			}
		}
		return true;
	}

	private Boolean validate(Tuple inner) throws JolRuntimeException {
		for (Filter filter : this.predicateFilters) {
			if (Boolean.FALSE.equals(filter.evaluate(inner))) {
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
	 * @throws PlannerException
	 */
	private void initFilters(Predicate predicate, Schema input)
	throws PlannerException {
		this.predicateFilters = new ArrayList<Filter>();
		this.joinFilters      = new ArrayList<JoinFilter>();

		Map<String, Integer> positions = new HashMap<String, Integer>();
		for (int position = 0; position < predicate.arguments().size(); position++ ) {
			Expression arg = predicate.argument(position);
			if (arg instanceof Variable) {
				Variable var = (Variable) arg;
				if (positions.containsKey(var.name())) {
					Integer prev = positions.get(var.name());
					predicateFilters.add(
							    new Filter(Filter.Operator.EQ,
								           new TableField(var.type(), prev),
								           new TableField(var.type(), position)));
				}
				else {
					positions.put(var.name(), position);
				}
			}
			else {
				predicateFilters.add(
						    new Filter(Filter.Operator.EQ,
						               new TableField(arg.type(), position),
						               arg.function(predicate.schema())));
			}
		}

		this.joinFilters = new ArrayList<JoinFilter>();
		for (Variable var : input.variables()) {
			if (predicate.schema().contains(var)) {
				TupleFunction<Object> o = var.function(input);
				TupleFunction<Object> i = var.function(predicate.schema());
				this.joinFilters.add(new JoinFilter(o, i));
			}
		}
	}

	/**
	 * Join the outer tuples with the inner tuples.
	 * @param outerTuples Outer tuples.
	 * @param innerTuples Inner tuples.
	 * @return Join result.
	 * @throws JolRuntimeException
	 */
	protected TupleSet join(TupleSet outerTuples, TupleSet innerTuples)
	throws JolRuntimeException {
		TupleSet result = new TupleSet();
		for (Tuple outer : outerTuples) {
			for (Tuple inner : innerTuples) {
				if (validate(inner) && validate(outer, inner)) {
					result.add(join(outer, inner));
				}
			}
		}
		return result;
	}

	private Tuple join(Tuple outer, Tuple inner) {
		Tuple result = outer.clone();
		for (Integer position : this.innerNonJoinPositions) {
			result.append(inner.value(position));
		}
		return result;
	}
}
