package p2.types.operator;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;

import p2.lang.plan.DontCare;
import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.exception.P2RuntimeException;
import p2.types.function.Accessor;
import p2.types.function.Filter;
import p2.types.function.TupleFunction;

public abstract class Join extends Operator {
	
	protected Predicate predicate;
	
	public Join(Predicate predicate) {
		super(predicate.program(), predicate.rule(), predicate.position());
		this.predicate = predicate;
	}
	
	@Override
	public Schema schema(Schema input) {
		return input.join(input.name() + " JOIN " + predicate.name(), predicate.schema());
	}

	@Override
	public Set<Variable> requires() {
		return predicate.requires();
	}
	
	protected List<Filter> filters() {
		List<Filter> filters = new ArrayList<Filter>();
		
		Hashtable<String, Variable> variables = new Hashtable<String, Variable>();
		for (p2.lang.plan.Expression arg : this.predicate) {
			if (arg instanceof Variable && !(arg instanceof DontCare)) {
				/* First determine the case of when we have variables named the same. 
				 * e.g., predicate(A, A, B, C, B) */
				if (variables.containsKey(arg.toString())) {
					TupleFunction<Comparable> lhs = variables.get(arg.toString()).function();
					TupleFunction<Comparable> rhs = arg.function();
					filters.add(new Filter(Filter.Operator.EQ, lhs, rhs));
				}
			}
			else {
				/* Some value will be produced that must then be compared to
				 * the current tuple value. */
				TupleFunction<Comparable> accessor  = new Accessor(arg.position(), arg.type());
				TupleFunction<Comparable> evaluator = arg.function();
				filters.add(new Filter(Filter.Operator.EQ, accessor, evaluator));
			}
		}
		return filters;
	}
}
