package p2.types.operator;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;

public class EventFilter extends Operator {
	private class Filter implements TupleFunction<Boolean> {
		private Integer position;
		
		private TupleFunction<Comparable> function;

		private Filter(Integer position,  TupleFunction<Comparable> function) {
			this.position = position;
			this.function = function;
		}

		public Boolean evaluate(Tuple tuple) throws P2RuntimeException {
			Comparable fvalue = function.evaluate(tuple);
			Comparable tvalue = tuple.value(position);
			
			return (fvalue == tvalue || fvalue.compareTo(tvalue) == 0);
		}

		public Class returnType() {
			return function.returnType();
		}
		
	}
	
	private Predicate predicate;
	
	private List<Filter> filters;
	

	public EventFilter(Predicate predicate) {
		super(predicate.program(), predicate.rule());
		this.predicate = predicate;
		this.filters = new ArrayList<Filter>();
		
		for (p2.lang.plan.Expression arg : predicate) {
			assert(arg.position() >= 0);
			
			if (!(arg instanceof Variable)) {
				this.filters.add(new Filter(arg.position(), arg.function()));
			}
		}
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws P2RuntimeException {
		if (filters.size() == 0) {
			return tuples;
		}
		else {
			TupleSet result = new TupleSet(tuples.name());
			
			for (Tuple tuple : tuples) {
				boolean valid = true;
				for (Filter filter : this.filters) {
					valid = filter.evaluate(tuple);
					if (!valid) break;
				}
				if (valid) result.add(tuple);
			}
			
			return result;
		}
	}

	@Override
	public Set<Variable> requires() {
		return this.predicate.requires();
	}

	@Override
	public Schema schema(Schema input) {
		return this.predicate.schema();
	}

	@Override
	public String toString() {
		return "EVENT FILTER " + predicate + ": FILTERS " + this.filters;
	}
	
	public int filters() {
		return this.filters.size();
	}

}
