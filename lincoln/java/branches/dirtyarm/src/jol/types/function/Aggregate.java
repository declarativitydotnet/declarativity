package jol.types.function;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.NavigableSet;
import java.util.TreeMap;

import jol.lang.plan.GenericAggregate;
import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;

public abstract class Aggregate<C extends Object> {

	public abstract void insert(Tuple tuple) throws JolRuntimeException;

	public abstract void delete(Tuple tuple) throws JolRuntimeException;

	public abstract C result();

	public abstract int size();

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
	public static final String LIST     = "list";
	public static final String UNION    = "union";



	public static Aggregate function(jol.lang.plan.Aggregate aggregate, Schema input)
	throws PlannerException {
		if (aggregate instanceof GenericAggregate) {
			return new Generic((GenericAggregate) aggregate, input);
		}
		else if (TOPK.equals(aggregate.functionName())) {
			return new TopBottomK((jol.lang.plan.TopK) aggregate, input);
		}
		else if (BOTTOMK.equals(aggregate.functionName())) {
			return new TopBottomK((jol.lang.plan.BottomK) aggregate, input);
		}
		else if (LIMIT.equals(aggregate.functionName())) {
			return new Limit((jol.lang.plan.Limit) aggregate, input);
		}
		else if (MIN.equals(aggregate.functionName())) {
			return new Min(aggregate, input);
		}
		else if (MAX.equals(aggregate.functionName())) {
			return new Max(aggregate, input);
		}
		else if (COUNT.equals(aggregate.functionName())) {
			return new Count(aggregate, input);
		}
		else if (AVG.equals(aggregate.functionName())) {
			return new Avg(aggregate, input);
		}
		else if (SUM.equals(aggregate.functionName())) {
			return new Sum(aggregate, input);
		}
		else if (SUMSTR.equals(aggregate.functionName())) {
			return new ConcatString(aggregate, input);
		}
		else if (TUPLESET.equals(aggregate.functionName())) {
			return new TupleCollection(aggregate, input);
		}
		else if (SET.equals(aggregate.functionName())) {
			return new Set(aggregate, input);
		}
		else if (LIST.equals(aggregate.functionName())) {
			return new List(aggregate, input);
		}
		else if (UNION.equals(aggregate.functionName())) {
			return new Union(aggregate, input);
		}
		throw new PlannerException("Unknown aggregate function " +
				aggregate.functionName());
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
			return jol.types.basic.BasicTupleSet.class;
		}
		else if (SET.equals(function) || UNION.equals(function)) {
			return HashSet.class;
		}
		else if (SUMSTR.equals(function)) {
			return java.lang.String.class;
		}
		else if (LIST.equals(function)) {
			return ArrayList.class;
		}
		return null;
	}

	public static class Limit extends Aggregate {
		private java.util.List<Object> values;
		private BasicTupleSet tuples;
		private java.util.List<Object> result;
		private TupleFunction<Object> accessor;

		public Limit(jol.lang.plan.Limit aggregate, Schema schema)
		throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		private void reset() {
			this.tuples = new BasicTupleSet();
			this.values = new ArrayList<Object>();
			this.result = null;
		}

		@Override
		public java.util.List<Object> result() {
			return this.result;
		}

		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				java.util.List result = (java.util.List) this.accessor.evaluate(tuple);
				Object value  = (Object) result.get(0);
				Number kConst = (Number) result.get(1);

				this.values.add(value);

				java.util.List<Object> resultValues = new ArrayList<Object>();
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
				BasicTupleSet tuples = this.tuples;
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
	}

	public static class TopBottomK<C extends Comparable<C>> extends Aggregate<Object> {
		private enum Type{TOP, BOTTOM};

		private TreeMap<C, ValueList<C>> values;
		private Type type;
		private BasicTupleSet tuples;
		private Object result;
		private TupleFunction<C> accessor;


		public TopBottomK(jol.lang.plan.TopK aggregate, Schema schema) throws PlannerException {
			this.type = Type.TOP;
			this.accessor = aggregate.function(schema);
			reset();
		}

		public TopBottomK(jol.lang.plan.BottomK aggregate, Schema schema) throws PlannerException {
			this.type = Type.BOTTOM;
			this.accessor = aggregate.function(schema);
			reset();
		}

		private void reset() {
			this.tuples = new BasicTupleSet();
			this.values = new TreeMap<C, ValueList<C>>();
			this.result = null;
		}

		@Override
		public Object result() {
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
					if (resultValues.size() == k) break;
					for (C element : this.values.get(key)) {
						resultValues.add(element);
						if (resultValues.size() == k) break;
					}
				}
				this.result = resultValues;
			}
		}

		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				BasicTupleSet tuples = this.tuples;
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
	}

	public static class Generic<C> extends Aggregate<Object> {
		private BasicTupleSet tuples;
		private C result;
		private GenericAggregate aggregate;
		private Schema schema;

		public Generic(GenericAggregate aggregate, Schema schema) {
			this.aggregate = aggregate;
			this.schema    = schema.clone();
			reset();
		}

		private void reset() {
			this.tuples = new BasicTupleSet();
			this.result = null;
		}

		@Override
		public Object result() {
			return this.result;
		}

		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			try {
				if (this.tuples.add(tuple)) {
					if (this.result == null) {
						this.result = (C) this.aggregate.function(schema).evaluate(tuple);
					}
					this.aggregate.function(this.result, schema).evaluate(tuple);
				}
			} catch (PlannerException e) {
				throw new JolRuntimeException(e.toString());
			}
		}

		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				BasicTupleSet tuples = this.tuples;
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
	}

	public static class Min<C extends Comparable<C>> extends Aggregate<C> {
		private BasicTupleSet tuples;
		private C result;
		private TupleFunction<C> accessor;

		public Min(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		private void reset() {
			this.result = null;
			this.tuples = new BasicTupleSet();
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
					BasicTupleSet tuples = this.tuples;
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
	}

	public static class Max<C extends Comparable<C>> extends Aggregate<C> {
		private BasicTupleSet tuples;
		private C result;
		private TupleFunction<C> accessor;

		public Max(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		private void reset() {
			this.result = null;
			this.tuples = new BasicTupleSet();
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
					BasicTupleSet tuples = this.tuples;
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
	}

	public static class Count extends Aggregate<Integer> {
		private BasicTupleSet tuples;
		private HashSet values;
		private TupleFunction<Object> accessor;

		public Count(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		private void reset() {
			this.tuples = new BasicTupleSet();
			this.values = new HashSet();
		}

		@Override
		public Integer result() {
			return this.values.size();
		}

		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				Object value = this.accessor.evaluate(tuple);
				if (value != null) this.values.add(value);
			}
		}

		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				Object value = this.accessor.evaluate(tuple);
				if (value != null) this.values.remove(value);
			}
		}

		@Override
		public int size() {
			return this.tuples.size();
		}
	}

	public static class Avg<C extends Comparable<C>> extends Aggregate<Float> {
		private BasicTupleSet tuples;
		private Float sum;
		private TupleFunction<C> accessor;

		public Avg(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		@Override
		public Float result() {
			return size() == 0 ? 0F : this.sum / (float) this.tuples.size();
		}

		public void reset() {
			this.tuples = new BasicTupleSet();
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
	}

	public static class Sum extends Aggregate<Float> {
		private BasicTupleSet tuples;
		private Float sum;
		private TupleFunction<Number> accessor;

		public Sum(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		@Override
		public Float result() {
			return this.sum;
		}

		public void reset() {
			this.tuples = new BasicTupleSet();
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
	}

	public static class ConcatString extends Aggregate<String> {
		private BasicTupleSet tuples;
		private StringBuilder buf;
		private TupleFunction<String> accessor;

		public ConcatString(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		@Override
		public String result() {
			if (this.buf == null)
				return null;

			return this.buf.toString();
		}

		public void reset() {
			this.tuples = new BasicTupleSet();
			this.buf = null;
		}

		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				String str = this.accessor.evaluate(tuple);
				if (this.buf == null) {
					this.buf = new StringBuilder(str);
				}
				else {
					this.buf.append(str);
				}
			}
		}

		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				BasicTupleSet tuples = this.tuples;
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
	}

	public static class TupleCollection extends Aggregate<TupleSet> {
		private BasicTupleSet tuples;
		private BasicTupleSet result;
		private TupleFunction<Tuple> accessor;

		public TupleCollection(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		private void reset() {
			this.tuples = new BasicTupleSet();
			this.result = new BasicTupleSet();
		}

		@Override
		public TupleSet result() {
			return this.result.clone();
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
	}

	public static class Set extends Aggregate<HashSet<Object>> {
		private BasicTupleSet tuples;
		private HashSet result;
		private TupleFunction<Tuple> accessor;

		public Set(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		private void reset() {
			this.tuples = new BasicTupleSet();
			this.result = new HashSet();
		}

		@Override
		public HashSet result() {
			return (HashSet) this.result.clone();
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
	}

	public static class List extends Aggregate<ArrayList<Object>> {
		private BasicTupleSet tuples;
		private ArrayList result;
		private TupleFunction<Tuple> accessor;

		public List(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		private void reset() {
			this.tuples = new BasicTupleSet();
			this.result = new ArrayList();
		}

		@Override
		public ArrayList result() {
			return (ArrayList) this.result.clone();
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
	}

	public static class Union extends Aggregate<HashSet<Object>> {
		private BasicTupleSet tuples;
		private HashSet result;
		private TupleFunction<Tuple> accessor;

		public Union(jol.lang.plan.Aggregate aggregate, Schema schema) throws PlannerException {
			this.accessor = aggregate.function(schema);
			reset();
		}

		private void reset() {
			this.tuples = new BasicTupleSet();
			this.result = new HashSet();
		}

		@Override
		public HashSet result() {
			return (HashSet) this.result.clone();
		}

		@Override
		public void insert(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.add(tuple)) {
				Object v = this.accessor.evaluate(tuple);
				if (v instanceof HashSet) {
					this.result.addAll((HashSet) v);
				}
				else {
					this.result.add(v);
				}
			}
		}

		@Override
		public void delete(Tuple tuple) throws JolRuntimeException {
			if (this.tuples.remove(tuple)) {
				Object v = this.accessor.evaluate(tuple);
				if (v instanceof HashSet) {
					this.result.removeAll((HashSet) v);
				}
				else {
					this.result.remove(v);
				}
			}
		}

		@Override
		public int size() {
			return this.tuples.size();
		}
	}
}
