package jol.types.function;

import java.util.HashSet;
import java.util.NavigableSet;
import java.util.TreeMap;

import jol.lang.plan.GenericAggregate;
import jol.types.basic.ComparableSet;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;

public abstract class Aggregate<C extends Comparable<C>> {
	
	public abstract void insert(Tuple tuple) throws JolRuntimeException;
	
	public abstract void delete(Tuple tuple) throws JolRuntimeException;
	
	public abstract Comparable result();
	
	public abstract int size();
	
	public abstract int position();
	
	public static final String TOPK     = "topk";
	public static final String BOTTOMK  = "bottomk";
	public static final String LIMIT    = "limit";
	public static final String MIN      = "min";
	public static final String MAX      = "max";
	public static final String SUM      = "sum";
	public static final String COUNT    = "count";
	public static final String AVG      = "avg";
	public static final String SUMSTR   = "sumstr";
	public static final String TUPLESET = "tupleset";
	public static final String SET      = "set";
	
	
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
		else if (LIMIT.equals(aggregate.functionName())) {
			return new Limit((jol.lang.plan.Limit)aggregate);
		}
		else if (MIN.equals(aggregate.functionName())) {
			return new Min(aggregate);
		}
		else if (MAX.equals(aggregate.functionName())) {
			return new Max(aggregate);
		}
		else if (COUNT.equals(aggregate.functionName())) {
			return new Count(aggregate);
		}
		else if (AVG.equals(aggregate.functionName())) {
			return new Avg(aggregate);
		}
		else if (SUM.equals(aggregate.functionName())) {
			return new Sum(aggregate);
		}
		else if (SUMSTR.equals(aggregate.functionName())) {
			return new ConcatString(aggregate);
		}
		else if (TUPLESET.equals(aggregate.functionName())) {
			return new TupleCollection(aggregate);
		}
		else if (SET.equals(aggregate.functionName())) {
			return new Set(aggregate);
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
		else if (AVG.equals(function) || SUM.equals(function)) {
			return Float.class;
		}
		else if (LIMIT.equals(function) ||
				 TOPK.equals(function) || 
				 BOTTOMK.equals(function)) {
			return ValueList.class;
		}
		else if (TUPLESET.equals(function)) {
			return jol.types.basic.TupleSet.class;
		}
		else if (SET.equals(function)) {
			return jol.types.basic.ComparableSet.class;
		}
		else if (SUMSTR.equals(function)) {
			return java.lang.String.class;
		}
		return null;
	}
	
	public static class Limit<C extends Comparable<C>> extends Aggregate<C> {
		private int position;
		private ValueList<C> values;
		private TupleSet tuples;
		private Comparable result;
		private TupleFunction<C> accessor;

		
		public Limit(jol.lang.plan.Limit aggregate) {
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		private void reset() {
			this.tuples = new TupleSet();
			this.values = new ValueList<C>();
			this.result = null;
		}
		
		@Override
		public Comparable result() {
			return this.result;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				ValueList result = (ValueList)this.accessor.evaluate(tuple);
				C      value  = (C) result.get(0);
				Number kConst = (Number) result.get(1);
						
				this.values.add(value);
				
				ValueList<C> resultValues = new ValueList<C>();
				int k = kConst.intValue();
				for (int i = 0; i < k; i++) {
					if (this.values.size() <= i) break;
					resultValues.add(this.values.get(i));
				}
				this.result = resultValues;
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				TupleSet tuples = this.tuples;
				reset();
				for (Tuple copy : tuples) {
					insert(copy);
				}
			}
		}
		
		@Override
		public int size() {
			return this.tuples.size();
		}

		@Override
		public int position() {
			return this.position;
		}
	}
	
	public static class TopBottomK<C extends Comparable<C>> extends Aggregate<C> {
		private enum Type{TOP, BOTTOM};
		
		private int position;
		private TreeMap<C, ValueList<C>> values;
		private Type type;
		private TupleSet tuples;
		private Comparable result;
		private TupleFunction<C> accessor;

		
		public TopBottomK(jol.lang.plan.TopK aggregate) {
			this.type = Type.TOP;
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		public TopBottomK(jol.lang.plan.BottomK aggregate) {
			this.type = Type.BOTTOM;
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		private void reset() {
			this.tuples = new TupleSet();
			this.values = new TreeMap<C, ValueList<C>>();
			this.result = null;
		}
		
		@Override
		public Comparable result() {
			return this.result;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				ValueList result = (ValueList)this.accessor.evaluate(tuple);
				C      value  = (C) result.get(0);
				Number kConst = (Number) result.get(1);
						
				if (!this.values.containsKey(value)) {
					this.values.put(value, new ValueList<C>());
				}
				this.values.get(value).add(value);
				
				ValueList<C> resultValues = new ValueList<C>();
				NavigableSet<C> keys = this.type == Type.TOP ?
						this.values.descendingKeySet() : this.values.navigableKeySet();
				int k = kConst.intValue();
				for (C key : keys) {
					if (k == 0) break;
					for (C element : this.values.get(key)) {
						if (k == 0) break;
						resultValues.add(element);
					}
				}
				this.result = resultValues;
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				TupleSet tuples = this.tuples;
				reset();
				for (Tuple copy : tuples) {
					insert(copy);
				}
			}
		}
		
		@Override
		public int size() {
			return this.tuples.size();
		}

		@Override
		public int position() {
			return this.position;
		}
	}
	
	public static class Generic<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private C result;
		private GenericAggregate aggregate;
		
		public Generic(GenericAggregate aggregate) {
			this.aggregate = aggregate;
			reset();
		}
		
		private void reset() {
			this.tuples = new TupleSet();
			this.result = null;
		}
		
		@Override
		public Comparable result() {
			return this.result;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				if (this.result == null) {
					this.result = (C)this.aggregate.function().evaluate(tuple);
				}
				this.aggregate.function(this.result).evaluate(tuple);
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				TupleSet tuples = this.tuples;
				reset();
				for (Tuple copy : tuples) {
					insert(copy);
				}
			}
		}
		
		@Override
		public int size() {
			return this.tuples.size();
		}

		@Override
		public int position() {
			return this.aggregate.position();
		}
	}
	
	public static class Min<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private C result;
		private TupleFunction<C> accessor;
		private int position;
		
		public Min(jol.lang.plan.Aggregate aggregate) {
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		private void reset() {
			this.result = null;
			this.tuples = new TupleSet();
		}
		
		@Override
		public C result() {
			return this.result;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				C value = accessor.evaluate(tuple);
				if (this.result == null || this.result.compareTo(value) > 0) {
					this.result = value;
				}
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				C value = accessor.evaluate(tuple);
				if (this.result.compareTo(value) == 0) {
					TupleSet tuples = this.tuples;
					reset();
					for (Tuple copy : tuples) {
						insert(copy);
					}
				}
			}
		}

		@Override
		public int size() {
			return this.tuples.size();
		}

		@Override
		public int position() {
			return this.position;
		}
	}
	
	public static class Max<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private C result;
		private TupleFunction<C> accessor;
		private int position;
		
		public Max(jol.lang.plan.Aggregate aggregate) {
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		private void reset() {
			this.result = null;
			this.tuples = new TupleSet();
		}
		
		@Override
		public C result() {
			return this.result;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				C value = accessor.evaluate(tuple);
				if (this.result == null || this.result.compareTo(value) < 0) {
					this.result = value;
				}
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.contains(tuple)) {
				this.tuples.remove(tuple);
				C value = accessor.evaluate(tuple);
				if (this.result.compareTo(value) == 0) {
					TupleSet tuples = this.tuples;
					reset();
					for (Tuple copy : tuples) {
						insert(copy);
					}
				}
			}
		}

		@Override
		public int size() {
			return this.tuples.size();
		}
		
		@Override
		public int position() {
			return this.position;
		}
	}
	
	public static class Count<C extends Comparable<C>> extends Aggregate<C>{
		private TupleSet tuples;
		private TupleFunction<C> accessor;
		private int position;
		
		public Count(jol.lang.plan.Aggregate aggregate) {
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		private void reset() {
			this.tuples = new TupleSet();
		}
		
		@Override
		public Comparable result() {
			return size();
		}
		
		@Override
		public void insert(Tuple tuple) {
			this.tuples.add(tuple);
		}
		
		@Override
		public void delete(Tuple tuple) {
			this.tuples.remove(tuple);
		}

		@Override
		public int size() {
			return this.tuples.size();
		}
		
		@Override
		public int position() {
			return this.position;
		}
	}
	
	public static class Avg<C extends Comparable<C>> extends Aggregate<C> {
		private TupleSet tuples;
		private Float sum;
		private TupleFunction<C> accessor;
		private int position;
		
		public Avg(jol.lang.plan.Aggregate aggregate) {
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		@Override
		public Float result() {
			return size() == 0 ? 0F : this.sum / (float) this.tuples.size();
		}
		
		public void reset() {
			this.tuples = new TupleSet();
			this.sum    = 0F;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				Number value = (Number) this.accessor.evaluate(tuple);
				this.sum += value.floatValue();
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				Number value = (Number) this.accessor.evaluate(tuple);
				this.sum -= value.floatValue();
			}
		}

		@Override
		public int size() {
			return this.tuples.size();
		}
		
		@Override
		public int position() {
			return this.position;
		}
	}
	
	public static class Sum extends Aggregate<Float> {
		private TupleSet tuples;
		private Float sum;
		private TupleFunction<Number> accessor;
		private int position;
		
		public Sum(jol.lang.plan.Aggregate aggregate) {
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		@Override
		public Float result() {
			return this.sum;
		}
		
		public void reset() {
			this.tuples = new TupleSet();
			this.sum    = 0F;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				Number value = (Number) this.accessor.evaluate(tuple);
				this.sum += value.floatValue();
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				Number value = (Number) this.accessor.evaluate(tuple);
				this.sum -= value.floatValue();
			}
		}

		@Override
		public int size() {
			return this.tuples.size();
		}
		
		@Override
		public int position() {
			return this.position;
		}
	}
	
	public static class ConcatString extends Aggregate<String> {
		private TupleSet tuples;
		private String result;
		private TupleFunction<String> accessor;
		private int position;
		
		public ConcatString(jol.lang.plan.Aggregate aggregate) {
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		@Override
		public String result() {
			return this.result;
		}
		
		public void reset() {
			this.tuples = new TupleSet();
			this.result = null;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				if (this.result == null) {
					this.result = this.accessor.evaluate(tuple);
				}
				else {
					this.result += this.accessor.evaluate(tuple);
				}
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				TupleSet tuples = this.tuples;
				reset();
				for (Tuple copy : tuples) {
					insert(copy);
				}
			}
		}

		@Override
		public int size() {
			return this.tuples.size();
		}
		
		@Override
		public int position() {
			return this.position;
		}
	}
	
	public static class TupleCollection extends Aggregate<Tuple> {
		private TupleSet tuples;
		private TupleSet result;
		private TupleFunction<Tuple> accessor;
		private int position;
		
		public TupleCollection(jol.lang.plan.Aggregate aggregate) {
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		private void reset() {
			this.tuples = new TupleSet();
			this.result = new TupleSet();
		}
		
		@Override
		public TupleSet result() {
			return this.result;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				this.result.add(this.accessor.evaluate(tuple));
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				this.result.remove(this.accessor.evaluate(tuple));
			}
		}

		@Override
		public int size() {
			return this.tuples.size();
		}
		
		@Override
		public int position() {
			return this.position;
		}
	}
	
	public static class Set extends Aggregate<Tuple> {
		private TupleSet tuples;
		private ComparableSet result;
		private TupleFunction<Tuple> accessor;
		private int position;
		
		public Set(jol.lang.plan.Aggregate aggregate) {
			this.accessor = aggregate.function();
			this.position = aggregate.position();
			reset();
		}
		
		private void reset() {
			this.tuples = new TupleSet();
			this.result = new ComparableSet();
		}
		
		@Override
		public ComparableSet result() {
			return this.result;
		}
		
		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				this.result.add(this.accessor.evaluate(tuple));
				System.err.println("TUPLE INSERTED SET AGG: " + tuple + " RESULT " + this.result);
			}
		}
		
		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				this.result.remove(this.accessor.evaluate(tuple));
				System.err.println("TUPLE REMOVED SET AGG: " + tuple + " RESULT " + this.result);
			}
		}

		@Override
		public int size() {
			return this.tuples.size();
		}
		
		@Override
		public int position() {
			return this.position;
		}
	}
	
}
