package p2.types.function;

import p2.types.basic.Tuple;

public abstract class Aggregate implements TupleFunction<Comparable> {
	
	public static final String MIN   = "min";
	public static final String MAX   = "max";
	public static final String COUNT = "count";
	public static final String AVG   = "avg";
	
	public static Aggregate function(String name, TupleFunction<Comparable> accessor) {
		if (MIN.equals(name)) {
			return new Min(accessor);
		}
		else if (MAX.equals(name)) {
			return new Max(accessor);
		}
		else if (COUNT.equals(name)) {
			return new Count(accessor);
		}
		else if (AVG.equals(name)) {
			return new Avg(accessor);
		}
		
		return null;
	}
	
	public abstract void reset();

	public static class Min extends Aggregate {
		private Comparable current;
		private TupleFunction<Comparable> accessor;
		
		public Min(TupleFunction<Comparable> accessor) {
			this.current = null;
			this.accessor = accessor;
		}
		
		public void reset() {
			this.current = null;
		}
		
		public Comparable evaluate(Tuple tuple) {
			Comparable value = accessor.evaluate(tuple);
			if (current == null || this.current.compareTo(value) > 0) {
				this.current = value;
			}
			return this.current;
		}

		public Class returnType() {
			return accessor.returnType();
		}
	}
	
	public static class Max extends Aggregate {
		private Comparable current;
		private TupleFunction<Comparable> accessor;
		
		public Max(TupleFunction<Comparable> accessor) {
			this.current = null;
			this.accessor = accessor;
		}
		
		public void reset() {
			this.current = null;
		}
		
		public Comparable evaluate(Tuple tuple) {
			Comparable value = accessor.evaluate(tuple);
			if (current == null || this.current.compareTo(value) < 0) {
				this.current = value;
			}
			return this.current;
		}

		public Class returnType() {
			return accessor == null ? Integer.class : accessor.returnType();
		}
	}
	
	public static class Count extends Aggregate {
		private Integer current;
		private TupleFunction<Comparable> accessor;
		
		public Count(TupleFunction<Comparable> accessor) {
			this.current = new Integer(0);
			this.accessor = accessor;
		}
		
		public void reset() {
			this.current = new Integer(0);
		}
		
		public Comparable evaluate(Tuple tuple) {
			this.current += 1;
			return this.current;
		}

		public Class returnType() {
			return Integer.class;
		}
	}
	
	public static class Avg extends Aggregate {
		private Float sum;
		private Float count;
		private TupleFunction<Comparable> accessor;
		
		public Avg(TupleFunction<Comparable> accessor) {
			this.sum = new Float(0);
			this.count = new Float(0);
			this.accessor = accessor;
		}
		
		public void reset() {
			this.sum = new Float(0);
			this.count = new Float(0);
		}
		
		public Comparable evaluate(Tuple tuple) {
			Number value = (Number) accessor.evaluate(tuple);
			this.sum += value.floatValue();
			this.count += 1.0;
			return this.sum / this.count;
		}

		public Class returnType() {
			return accessor.returnType();
		}
	}
}
