package jol.types.operator;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.lang.plan.Expression;
import jol.lang.plan.GenericAggregate;
import jol.lang.plan.Predicate;
import jol.lang.plan.Variable;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;
import jol.types.function.TupleFunction;
import jol.core.Runtime;

public class Projection extends Operator {
	
	private Schema schema;
	
	List<TupleFunction<Comparable>> accessors = new ArrayList<TupleFunction<Comparable>>();
	
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
