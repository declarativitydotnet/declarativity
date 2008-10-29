package jol.types.operator;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.lang.plan.Expression;
import jol.lang.plan.GenericAggregate;
import jol.lang.plan.Limit;
import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;
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
	
	/**
	 * Create a new projection operator
	 * @param context THe runtime context.
	 * @param predicate The predicate whose schema we project to.
	 */
	public Projection(Runtime context, Predicate predicate) {
		super(context, predicate.program(), predicate.rule());
		
		List<Variable>   projection = new ArrayList<Variable>();
		List<Variable>   variables  = predicate.schema().variables();
		List<Expression> arguments  = predicate.arguments();
		
		for (int i = 0; i < variables.size(); i++) {
			Variable var = variables.get(i);
			if (var instanceof GenericAggregate) {
				for (Variable sub : var.variables()) {
					accessors.add(sub.function());
					projection.add(sub);
				}
			}
			else {
				accessors.add(arguments.get(i).function());
				projection.add(var);
				
				if (var instanceof Limit) {
					Limit limit = (Limit) var;
					if (limit.kVar() != null) {
						accessors.add(limit.kVar().function());
						projection.add(limit.kVar());
					}
				}
			}
		}
		
		this.schema = new Schema(predicate.name(), projection);
	}
	
	@Override
	public String toString() {
		return "PROJECTION [" + schema() + "]";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		TupleSet result = new TupleSet(schema().name());
		for (Tuple tuple : tuples) {
			List<Comparable> values = new ArrayList<Comparable>();
			for (TupleFunction<Comparable> accessor : accessors) {
				values.add(accessor.evaluate(tuple));
			}
			Tuple projection = new Tuple(values);
			projection.schema(schema());
			result.add(projection);
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
