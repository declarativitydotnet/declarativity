package p2.types.function;

import p2.types.basic.Tuple;
import p2.types.exception.RuntimeException;

public abstract class Aggregate implements TupleFunction<Comparable> {
	
	public static final String MIN   = "min";
	public static final String MAX   = "max";
	public static final String COUNT = "count";
	public static final String AVG   = "avg";
	
	public abstract Tuple result();
	
	public static Aggregate function(p2.lang.plan.Aggregate agg) {
		if (MIN.equals(agg.functionName())) {
			return new Min(agg);
		}
		else if (MAX.equals(agg.functionName())) {
			return new Max(agg);
		}
		else if (COUNT.equals(agg.functionName())) {
			return new Count(agg);
		}
		else if (AVG.equals(agg.functionName())) {
			return new Avg(agg);
		}
		
		return null;
	}
	
	public static class Min extends Aggregate {
		private Tuple result;
		private Comparable current;
		private TupleFunction<Comparable> accessor;
		private p2.lang.plan.Aggregate variable;
		
		public Min(p2.lang.plan.Aggregate variable) {
			this.result = null;
			this.current = null;
			this.accessor = variable.function();
			this.variable = variable;
		}
		
		public Tuple result() {
			return this.result.clone();
		}
		
		public Comparable evaluate(Tuple tuple) throws RuntimeException {
			Comparable value = accessor.evaluate(tuple);
			if (current == null || this.current.compareTo(value) > 0) {
				this.current = value;
				this.result = tuple;
				return value;
			}
			return null;
		}

		public Class returnType() {
			return accessor.returnType();
		}
	}
	
	public static class Max extends Aggregate {
		private Tuple result;
		private Comparable current;
		private TupleFunction<Comparable> accessor;
		private p2.lang.plan.Aggregate variable;
		
		public Max(p2.lang.plan.Aggregate variable) {
			this.result = null;
			this.current = null;
			this.accessor = variable.function();
			this.variable = variable;
		}
		
		public Tuple result() {
			return this.result.clone();
		}
		
		public Comparable evaluate(Tuple tuple) throws RuntimeException {
			Comparable value = accessor.evaluate(tuple);
			if (current == null || this.current.compareTo(value) < 0) {
				this.current = value;
				this.result = tuple;
				return value;
			}
			return null;
		}

		public Class returnType() {
			return accessor.returnType();
		}
	}
	
	public static class Count extends Aggregate {
		private Tuple result;
		private Integer current;
		private TupleFunction<Comparable> accessor;
		private p2.lang.plan.Aggregate variable;
		
		public Count(p2.lang.plan.Aggregate variable) {
			this.result = null;
			this.current = new Integer(0);
			this.accessor = variable.function();
			this.variable = variable;
		}
		
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				int position = this.variable.position();
				this.result.value(position, this.current);
				return this.result;
			}
			return null;
		}
		
		public Comparable evaluate(Tuple tuple) {
			this.result = tuple;
			this.current += 1;
			return this.current;
		}

		public Class returnType() {
			return Integer.class;
		}
	}
	
	public static class Avg extends Aggregate {
		private Tuple result;
		private Float sum;
		private Float count;
		private TupleFunction<Comparable> accessor;
		private p2.lang.plan.Aggregate variable;
		
		public Avg(p2.lang.plan.Aggregate variable) {
			this.result = null;
			this.sum = new Float(0);
			this.count = new Float(0);
			this.accessor = variable.function();
			this.variable = variable;
		}
		
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				int position = this.variable.position();
				this.result.value(position, this.sum / this.count);
				return this.result;
			}
			return null;
		}
		
		public Comparable evaluate(Tuple tuple) throws RuntimeException {
			this.result = tuple;
			Number value = (Number) this.accessor.evaluate(tuple);
			this.sum += value.floatValue();
			this.count += 1.0;
			return this.sum / this.count;
		}

		public Class returnType() {
			return Float.class;
		}
	}
}
