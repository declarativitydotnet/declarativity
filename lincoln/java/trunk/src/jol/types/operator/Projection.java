package jol.types.operator;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.core.Runtime;
import jol.lang.plan.AggregateVariable;
import jol.lang.plan.DontCare;
import jol.lang.plan.Expression;
import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;

/**
 * A Projection operator "projects" the input tuples onto a subset of
 * attributes defined by the given predicate (head) schema.
 */
public class Projection extends Operator {

	private Predicate predicate;

	/** The field accessors of the projection. */
	private List<TupleFunction<Comparable>> accessors;

	private Schema schema;

	/**
	 * Create a new projection operator
	 * @param context The runtime context.
	 * @param predicate The predicate whose schema we project to.
	 * @throws JolRuntimeException
	 */
	public Projection(Runtime context, Predicate predicate, Schema input) throws PlannerException {
		super(context, predicate.program(), predicate.rule());
		this.predicate = predicate;
		this.accessors = new ArrayList<TupleFunction<Comparable>>();
		this.schema    = new Schema(predicate.name());

		for (int i = 0; i < predicate.arguments().size(); i++) {
			Expression argument = predicate.arguments().get(i).clone();
			Set<Variable> variables = argument.variables();
			for (Variable var : variables) {
			    // Skip "*" variables
			    if (var.name().equals(AggregateVariable.STAR))
			        continue;

				int position = input.position(var.name());
				if (position < 0) {
					throw new PlannerException("Unknown variable " + var +
							" in input schema " + input);
				}
				var.position(position);
			}
			if (argument instanceof Variable) {
				this.schema.append((Variable) argument);
			}
			else {
				this.schema.append(new DontCare(argument.type()));
			}

			accessors.add(argument.function());
		}
	}

	@Override
	public String toString() {
		return "PROJECTION [" + this.predicate + "]";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		TupleSet result = new TupleSet(predicate.name());
		for (Tuple tuple : tuples) {
			try {
				List<Comparable> values = new ArrayList<Comparable>();
				for (TupleFunction<Comparable> accessor : accessors) {
					values.add(accessor.evaluate(tuple));
				}
				Tuple projection = new Tuple(values);
				projection.schema(schema());
				result.add(projection);
			} catch (Throwable e) {
				System.err.println("PROJECTION ERROR " + this.predicate
						+ ": PROGRAM " + this.program + " RULE " + this.rule);
				System.exit(0);
			}
		}
		return result;
	}

	@Override
	public Schema schema() {
		return this.schema;
	}

	@Override
	public Set<Variable> requires() {
		return new HashSet<Variable>(predicate.requires());
	}
}
