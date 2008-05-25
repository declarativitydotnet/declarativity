package p2.types.function;

import p2.types.basic.Tuple;
import p2.types.exception.P2RuntimeException;

public abstract class Aggregate implements TupleFunction<Comparable> {
	
	private static class Accessor implements TupleFunction<Comparable> {
		private Integer position;
		
		private Class type;
		
		public Accessor(p2.lang.plan.Aggregate aggregate) {
			this.position = aggregate.position();
			this.type     = aggregate.type();
		}
		
		public Integer position() {
			return this.position;
		}

		public Comparable evaluate(Tuple tuple) throws P2RuntimeException {
			return tuple.value(this.position);
		}

		public Class returnType() {
			return this.type;
		}
		
	}
	
	public static final String MIN   = "min";
	public static final String MAX   = "max";
	public static final String COUNT = "count";
	public static final String AVG   = "avg";
	
	public abstract Tuple result();
	
	public static Aggregate function(p2.lang.plan.Aggregate aggregate) {
		if (MIN.equals(aggregate.functionName())) {
			return new Min(new Accessor(aggregate));
		}
		else if (MAX.equals(aggregate.functionName())) {
			return new Max(new Accessor(aggregate));
		}
		else if (COUNT.equals(aggregate.functionName())) {
			return new Count(new Accessor(aggregate));
		}
		else if (AVG.equals(aggregate.functionName())) {
			return new Avg(new Accessor(aggregate));
		}
		
		return null;
	}
	
	public static class Min extends Aggregate {
		private Tuple result;
		private Comparable current;
		private Accessor accessor;
		
		public Min(Accessor accessor) {
			this.result = null;
			this.current = null;
			this.accessor = accessor;
		}
		
		public Tuple result() {
			return this.result.clone();
		}
		
		public Comparable evaluate(Tuple tuple) throws P2RuntimeException {
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
		private Accessor accessor;
		
		public Max(Accessor accessor) {
			this.result = null;
			this.current = null;
			this.accessor = accessor;
		}
		
		public Tuple result() {
			return this.result.clone();
		}
		
		public Comparable evaluate(Tuple tuple) throws P2RuntimeException {
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
		private Accessor accessor;
		
		public Count(Accessor accessor) {
			this.result = null;
			this.current = new Integer(0);
			this.accessor = accessor;
		}
		
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				this.result.value(accessor.position(), this.current);
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
		private Accessor accessor;
		
		public Avg(Accessor accessor) {
			this.result = null;
			this.sum = new Float(0);
			this.count = new Float(0);
			this.accessor = accessor;
		}
		
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				this.result.value(accessor.position(), this.sum / this.count);
				return this.result;
			}
			return null;
		}
		
		public Comparable evaluate(Tuple tuple) throws P2RuntimeException {
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
