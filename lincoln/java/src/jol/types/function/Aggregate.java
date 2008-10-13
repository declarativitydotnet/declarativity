package jol.types.function;

import jol.lang.plan.GenericAggregate;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.P2RuntimeException;

public abstract class Aggregate<C extends Comparable<C>> {
	
	public abstract Tuple insert(Tuple tuple) throws P2RuntimeException;
	
	public abstract Tuple delete(Tuple tuple) throws P2RuntimeException;
	
	public abstract Tuple result();
	
	public abstract TupleSet tuples();
	
	public abstract Class returnType();
	
	private static class Accessor<C extends Comparable<C>> implements TupleFunction<C> {
		private Integer position;
		
		private Class type;
		
		public Accessor(jol.lang.plan.Aggregate aggregate) {
			this.position = aggregate.position();
			this.type     = aggregate.type();
		}
		
		public Integer position() {
			return this.position;
		}

		public C evaluate(Tuple tuple) throws P2RuntimeException {
			return (C) tuple.value(this.position);
		}

		public Class returnType() {
			return this.type;
		}
	}
	
	public static final String MIN      = "min";
	public static final String MAX      = "max";
	public static final String COUNT    = "count";
	public static final String AVG      = "avg";
	public static final String SUMSTR   = "sumstr";
	public static final String TUPLESET = "tupleset";
	
	public static Aggregate function(jol.lang.plan.Aggregate aggregate) {
		if (aggregate instanceof GenericAggregate) {
			return new Generic((GenericAggregate) aggregate);
		}
		else if (MIN.equals(aggregate.functionName())) {
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
		else if (SUMSTR.equals(aggregate.functionName())) {
			return new ConcatString(new Accessor(aggregate));
		}
		else if (TUPLESET.equals(aggregate.functionName())) {
			return new TupleCollection(new Accessor(aggregate));
		}
		
		return null;
	}
	
	public static Class type(String function, Class type) {
		if (MIN.equals(function) || MAX.equals(function)) {
			return type;
		}
		else if (COUNT.equals(function)) {
			return Integer.class;
		}
		else if (AVG.equals(function)) {
			return Float.class;
		}
		else if (TUPLESET.equals(function)) {
			return jol.types.basic.TupleSet.class;
		}
		return null;
	}
	
	public static class Generic<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private Tuple result;
		private Comparable<?> current;
		private GenericAggregate aggregate;
		
		public Generic(GenericAggregate aggregate) {
			this.aggregate = aggregate;
			reset();
		}
		
		private void reset() {
			this.tuples = new TupleSet();
			this.result = null;
			this.current = null;
		}
		
		public Tuple result() {
			return this.result == null ? null : this.result.clone();
		}
		
		public Tuple insert(Tuple tuple) throws P2RuntimeException {
			if (this.current == null) {
				this.current = (Comparable<?>) this.aggregate.function().evaluate(tuple);
			}
			this.aggregate.function(this.current).evaluate(tuple);
			this.tuples.add(tuple);
			this.result = tuple.clone();
			this.result.value(this.aggregate.position(), this.current);
			return this.result;
		}
		
		public Tuple delete(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.remove(tuple)) {
				TupleSet tuples = this.tuples;
				reset();
				Tuple last = null;
				for (Tuple copy : tuples) {
					last = insert(copy);
				}
				return last;
			}
			return null;
		}

		public Class returnType() {
			return this.aggregate.type();
		}

		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
	
	public static class Min<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private Tuple result;
		private C current;
		private Accessor<C> accessor;
		
		public Min(Accessor<C> accessor) {
			this.accessor = accessor;
			reset();
		}
		
		private void reset() {
			this.result = null;
			this.current = null;
			this.tuples = new TupleSet();
		}
		
		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
		
		@Override
		public Tuple result() {
			return this.result == null ? null : this.result.clone();
		}
		
		@Override
		public Tuple insert(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.add(tuple)) {
				C value = accessor.evaluate(tuple);
				if (current == null || this.current.compareTo(value) > 0) {
					this.current = value;
					this.result = tuple;
					return result;
				}
			}
			return null;
		}
		
		@Override
		public Tuple delete(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.remove(tuple)) {
				C  value = accessor.evaluate(tuple);
				if (this.current.compareTo(value) == 0) {
					TupleSet tuples = this.tuples;
					reset();
					for (Tuple copy : tuples) {
						insert(copy);
					}
					return result();
				}
			}
			return null;
		}

		@Override
		public Class returnType() {
			return accessor.returnType();
		}
	}
	
	public static class Max<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private Tuple result;
		private C current;
		private Accessor<C> accessor;
		
		public Max(Accessor<C> accessor) {
			this.result = null;
			this.current = null;
			this.tuples = new TupleSet();
			this.accessor = accessor;
		}
		
		public Tuple result() {
			return this.result == null ? null : this.result.clone();
		}
		
		public Tuple insert(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.add(tuple)) {
				C value = accessor.evaluate(tuple);
				if (current == null || this.current.compareTo(value) < 0) {
					this.current = value;
					this.result = tuple;
					return result;
				}
			}
			return null;
		}
		
		public Tuple delete(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.contains(tuple)) {
				this.tuples.remove(tuple);
				C value = accessor.evaluate(tuple);
				if (this.current.compareTo(value) == 0) {
					TupleSet tuples = this.tuples;
					this.tuples = new TupleSet();
					this.current = null;
					this.result  = null;
					for (Tuple copy : tuples) {
						insert(copy);
					}
					return result();
				}
			}
			return null;
		}

		public Class returnType() {
			return accessor.returnType();
		}
		
		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
	
	public static class Count<C extends Comparable<C>> extends Aggregate<C>{
		private TupleSet tuples;
		private Tuple result;
		private Accessor<C> accessor;
		
		public Count(Accessor<C> accessor) {
			this.tuples = new TupleSet();
			this.result = null;
			this.accessor = accessor;
		}
		
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				this.result.value(accessor.position(), this.tuples.size());
				return this.result;
			}
			return null;
		}
		
		public Tuple insert(Tuple tuple) {
			if (this.tuples.add(tuple)) {
				this.result = tuple;
				return result();
			}
			return null;
		}
		
		public Tuple delete(Tuple tuple) {
			this.result = tuple;
			this.tuples.remove(tuple);
			return result();
		}

		public Class returnType() {
			return Integer.class;
		}
		
		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
	
	public static class Avg<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private Tuple result;
		private Float sum;
		private Accessor<C> accessor;
		
		public Avg(Accessor<C> accessor) {
			this.accessor = accessor;
			reset();
		}
		
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				this.result.value(accessor.position(), this.sum / (float)this.tuples.size());
				return this.result;
			}
			return null;
		}
		
		public void reset() {
			this.tuples = new TupleSet();
			this.result = null;
			this.sum = 0F;
		}
		
		public Tuple insert(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.add(tuple)) {
				this.result = tuple;
				Number value = (Number) this.accessor.evaluate(tuple);
				this.sum += value.floatValue();
				return result();
			}
			return null;
		}
		
		public Tuple delete(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.remove(tuple)) {
				this.tuples.add(tuple);
				this.result = tuple;
				Number value = (Number) this.accessor.evaluate(tuple);
				this.sum -= value.floatValue();
				return result();
			}
			return null;
		}

		public Class returnType() {
			return Float.class;
		}
		
		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
	
	public static class ConcatString<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private Tuple result;
		private String current;
		private Accessor<C> accessor;
		
		public ConcatString(Accessor<C> accessor) {
			this.accessor = accessor;
			reset();
		}
		
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				this.result.value(accessor.position(), this.current);
				return this.result;
			}
			return null;
		}
		
		public void reset() {
			this.tuples = new TupleSet();
			this.result = null;
			this.current = null;
		}
		
		public Tuple insert(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.add(tuple)) {
				this.result = tuple;
				if (this.current == null) {
					this.current = (String) this.accessor.evaluate(tuple);
				}
				else {
					this.current += (String) this.accessor.evaluate(tuple);
				}
				return result();
			}
			return null;
		}
		
		public Tuple delete(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.remove(tuple)) {
				TupleSet tuples = this.tuples;
				reset();
				for (Tuple copy : tuples) {
					insert(copy);
				}
				return result();
			}
			return null;
		}

		public Class returnType() {
			return String.class;
		}
		
		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
	
	public static class TupleCollection<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private Tuple    result;
		private TupleSet nestedSet;
		private Accessor<C> accessor;
		
		public TupleCollection(Accessor<C> accessor) {
			this.accessor = accessor;
			this.tuples = new TupleSet();
			this.nestedSet = new TupleSet();
			this.result = null;
		}
		
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				this.result.value(accessor.position(), this.nestedSet.clone());
				return this.result;
			}
			return null;
		}
		
		public Tuple insert(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.add(tuple)) {
				this.result = tuple;
				this.nestedSet.add((Tuple) this.accessor.evaluate(tuple));
				return result();
			}
			return null;
		}
		
		public Tuple delete(Tuple tuple) throws P2RuntimeException {
			if (this.tuples.remove(tuple)) {
				this.result = tuple;
				this.nestedSet.remove((Tuple) this.accessor.evaluate(tuple));
				return result();
			}
			return null;
		}

		public Class returnType() {
			return TupleSet.class;
		}
		
		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
}
