package p2.types.operator;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import p2.lang.plan.Expression;
import p2.lang.plan.GenericAggregate;
import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;


public class Projection extends Operator {
	
	private Schema schema;
	
	List<TupleFunction<Comparable>> accessors = new ArrayList<TupleFunction<Comparable>>();
	
	public Projection(Predicate predicate) {
		super(predicate.program(), predicate.rule());
		
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
			}
		}
		
		this.schema = new Schema(predicate.name(), projection);
	}
	
	@Override
	public String toString() {
		return "PROJECTION [" + schema() + "]";
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
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
