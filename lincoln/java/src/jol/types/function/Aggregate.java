package jol.types.function;

import java.util.Iterator;
import java.util.NavigableMap;
import java.util.NavigableSet;
import java.util.SortedSet;
import java.util.TreeMap;
import java.util.TreeSet;

import jol.lang.plan.GenericAggregate;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;

public abstract class Aggregate<C extends Comparable<C>> {
	
	public abstract Tuple insert(Tuple tuple) throws JolRuntimeException;
	
	public abstract Tuple delete(Tuple tuple) throws JolRuntimeException;
	
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

		public C evaluate(Tuple tuple) throws JolRuntimeException {
			return (C) tuple.value(this.position);
		}

		public Class returnType() {
			return this.type;
		}
	}
	
	public static final String TOPK     = "topk";
	public static final String BOTTOMK  = "bottomk";
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
		else if (TOPK.equals(aggregate.functionName())) {
			return new TopBottomK((jol.lang.plan.TopK)aggregate);
		}
		else if (BOTTOMK.equals(aggregate.functionName())) {
			return new TopBottomK((jol.lang.plan.BottomK)aggregate);
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
			return new ConcatString(new Accessor<String>(aggregate));
		}
		else if (TUPLESET.equals(aggregate.functionName())) {
			return new TupleCollection(new Accessor<Tuple>(aggregate));
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
	
	public static class TopBottomK<C extends Comparable<C>> extends Aggregate<C> {
		private enum Type{TOP, BOTTOM};
		
		private Integer k;
		private TreeMap<C, ValueList<C>> values;
		private Type type;
		private TupleSet tuples;
		private Tuple result;
		private Accessor<C> accessor;

		
		public TopBottomK(jol.lang.plan.TopK aggregate) {
			this.type = Type.TOP;
			this.k = aggregate.k();
			this.accessor = new Accessor<C>(aggregate);
			reset();
		}
		
		public TopBottomK(jol.lang.plan.BottomK aggregate) {
			this.type = Type.BOTTOM;
			this.k = aggregate.k();
			this.accessor = new Accessor<C>(aggregate);
			reset();
		}
		
		private void reset() {
			this.tuples = new TupleSet();
			this.values = new TreeMap<C, ValueList<C>>();
			this.result = null;
		}
		
		@Override
		public Tuple result() {
			return this.result == null ? null : this.result.clone();
		}
		
		@Override
		public Tuple insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				C value = accessor.evaluate(tuple);
				if (!this.values.containsKey(value)) {
					this.values.put(value, new ValueList<C>());
				}
				this.values.get(value).add(value);
				
				ValueList<C> resultValues = new ValueList<C>();
				NavigableSet<C> keys = this.type == Type.TOP ?
						this.values.descendingKeySet() : this.values.navigableKeySet();
				int k = this.k;
				for (C key : keys) {
					if (k == 0) break;
					for (C element : this.values.get(key)) {
						if (k == 0) break;
						resultValues.add(element);
					}
				}
				this.result = tuple.clone();
				this.result.value(this.accessor.position(), resultValues);
				return this.result;
			}
			return null;
		}
		
		@Override
		public Tuple delete(Tuple tuple) throws JolRuntimeException {
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

		@Override
		public Class returnType() {
			return ValueList.class;
		}

		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
	
	public static class Generic<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private Tuple result;
		private C current;
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
		
		@Override
		public Tuple result() {
			return this.result == null ? null : this.result.clone();
		}
		
		@Override
		public Tuple insert(Tuple tuple) throws JolRuntimeException {
			if (this.current == null) {
			    this.current = (C)this.aggregate.function().evaluate(tuple);
			}
			this.aggregate.function(this.current).evaluate(tuple);
			this.tuples.add(tuple);
			this.result = tuple.clone();
			this.result.value(this.aggregate.position(), this.current);
			return this.result;
		}
		
		@Override
		public Tuple delete(Tuple tuple) throws JolRuntimeException {
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

		@Override
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
		public Tuple insert(Tuple tuple) throws JolRuntimeException {
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
		public Tuple delete(Tuple tuple) throws JolRuntimeException {
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
		
		@Override
		public Tuple result() {
			return this.result == null ? null : this.result.clone();
		}
		
		@Override
		public Tuple insert(Tuple tuple) throws JolRuntimeException {
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
		
		@Override
		public Tuple delete(Tuple tuple) throws JolRuntimeException {
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

		@Override
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
		
		@Override
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				this.result.value(accessor.position(), this.tuples.size());
				return this.result;
			}
			return null;
		}
		
		@Override
		public Tuple insert(Tuple tuple) {
			if (this.tuples.add(tuple)) {
				this.result = tuple;
				return result();
			}
			return null;
		}
		
		@Override
		public Tuple delete(Tuple tuple) {
			this.result = tuple;
			this.tuples.remove(tuple);
			return result();
		}

		@Override
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
		
		@Override
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
		
		@Override
		public Tuple insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				this.result = tuple;
				Number value = (Number) this.accessor.evaluate(tuple);
				this.sum += value.floatValue();
				return result();
			}
			return null;
		}
		
		@Override
		public Tuple delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				this.tuples.add(tuple);
				this.result = tuple;
				Number value = (Number) this.accessor.evaluate(tuple);
				this.sum -= value.floatValue();
				return result();
			}
			return null;
		}

		@Override
		public Class returnType() {
			return Float.class;
		}
		
		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
	
	public static class ConcatString extends Aggregate<String> {
		private TupleSet tuples;
		private Tuple result;
		private String current;
		private Accessor<String> accessor;
		
		public ConcatString(Accessor<String> accessor) {
			this.accessor = accessor;
			reset();
		}
		
		@Override
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
		
		@Override
		public Tuple insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				this.result = tuple;
				if (this.current == null) {
					this.current = this.accessor.evaluate(tuple);
				}
				else {
					this.current += this.accessor.evaluate(tuple);
				}
				return result();
			}
			return null;
		}
		
		@Override
		public Tuple delete(Tuple tuple) throws JolRuntimeException {
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

		@Override
		public Class returnType() {
			return String.class;
		}
		
		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
	
	public static class TupleCollection extends Aggregate<Tuple> {
		private TupleSet tuples;
		private Tuple    result;
		private TupleSet nestedSet;
		private Accessor<Tuple> accessor;
		
		public TupleCollection(Accessor<Tuple> accessor) {
			this.accessor = accessor;
			this.tuples = new TupleSet();
			this.nestedSet = new TupleSet();
			this.result = null;
		}
		
		@Override
		public Tuple result() {
			if (this.result != null) {
				this.result = this.result.clone();
				this.result.value(accessor.position(), this.nestedSet.clone());
				return this.result;
			}
			return null;
		}
		
		@Override
		public Tuple insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				this.result = tuple;
				this.nestedSet.add(this.accessor.evaluate(tuple));
				return result();
			}
			return null;
		}
		
		@Override
		public Tuple delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				this.result = tuple;
				this.nestedSet.remove(this.accessor.evaluate(tuple));
				return result();
			}
			return null;
		}

		@Override
		public Class returnType() {
			return TupleSet.class;
		}
		
		@Override
		public TupleSet tuples() {
			return this.tuples.clone();
		}
	}
}
