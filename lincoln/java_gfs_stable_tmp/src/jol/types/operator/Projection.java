package jol.types.operator;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.lang.plan.Expression;
import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;
import jol.types.table.TableName;
import jol.core.Runtime;

/**
 * A Projection operator "projects" the input tuples onto a subset of
 * attributes defined by the given predicate (head) schema.
 */
public class Projection extends Operator {
	
	/** The projection schema. */
	private Schema schema;
	
	/** The field accessors of the projection. */
	List<TupleFunction<Comparable>> accessors = new ArrayList<TupleFunction<Comparable>>();
	
	private TableName name;
	
	/**
	 * Create a new projection operator
	 * @param context THe runtime context.
	 * @param predicate The predicate whose schema we project to.
	 * @throws JolRuntimeException 
	 */
	public Projection(Runtime context, Predicate predicate) throws PlannerException {
		super(context, predicate.program(), predicate.rule());
		this.name = predicate.name();
		
		List<Variable> variables = new ArrayList<Variable>();
		List<Expression> arguments = predicate.arguments();
		List<Variable> predicateVariables = predicate.schema().variables();
		
		for (int i = 0; i < arguments.size(); i++) {
			Expression argument = arguments.get(i);
			if (argument instanceof Variable) {
				for (Variable var : ((Variable)argument).variables()) {
					accessors.add(var.function());
					variables.add(var);
				}
			}
			else {
				if (predicateVariables.size() <= i) {
					// Fatal error!
					throw new PlannerException("Projection error: " +
							"NOT ENOUGH VARIABLES IN PREDICATE " + predicate.toString() +
					        " -- PREDICATE VARIABLES: " + predicateVariables.toString() +
					        " -- PREDICATE ARGUMENTS: " + arguments.toString());
				}
				accessors.add(argument.function());
				variables.add(predicateVariables.get(i));
			}
		}
		this.schema = new Schema(predicate.name(), variables);
	}
	
	@Override
	public String toString() {
		return "PROJECTION [" + schema() + "]";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		TupleSet result = new TupleSet(schema().name());
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
				System.err.println("PROJECTION ERROR " + this.name
						+ ": PROGRAM " + this.program + " RULE " + this.rule + 
						" -- SCHEMA " + this.schema + " tuple " + tuple + 
						" TUPLE SCHEMA " + tuple.schema());
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
		return new HashSet<Variable>(schema().variables());
	}
}
