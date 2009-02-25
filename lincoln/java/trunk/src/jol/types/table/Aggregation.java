package jol.types.table;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import jol.core.Runtime;
import jol.lang.plan.Expression;
import jol.lang.plan.Predicate;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.exception.UpdateException;
import jol.types.function.Aggregate;

/**
 * A table aggregation.
 *
 *  Table aggregations maintain all tuples inserted into the table and
 *  remove those tuples on deletion. However, the tuples that are made
 *  visible to the outside world are the aggregate values generated
 *  by the set of resident tuples. A table aggregation is created if a
 *  predicate referring to the table contains an aggregation. The
 *  aggregate function is registered with this table object. The
 *  aggregate function determines the value of the aggregate value.
 *  Deletions to this table may cause new tuples to appear. For instance
 *  if the aggregation function is a min and the tuple containing the
 *  min value is deleted then a new tuple could appear with a different
 *  (greater) min valued aggregate. The semantics of table aggregations
 *  are handled by the {@link jol.core.Driver.Flusher#insert(Tuple)}
 *  method.
 *
 *  TYPE: Aggregations type can be either materialized {@link Table.Type#TABLE} or
 *  event {@link Table.Type#EVENT}. An event type aggregation performs
 *  its aggregation only on the set of tuples passed into the {@link #insert(Tuple)} method.
 *  A materialized type aggregation maintains aggregate values over a stored set of
 *  tuples. Storage insert/delete semantics follow that of {@link jol.types.table.BasicTable}.
 *
 *  PRIMARY KEY: The primary key of an Aggregate table is the GroupBy columns identified
 *  by the predicate containing the aggregation.
 */
public class Aggregation<C extends Comparable<C>> extends Table {
	/** Stores base tuples in aggregate functions. */
	private Map<Tuple, List<Aggregate<C>>> aggregateFunctions;

	private List<Aggregate<C>> singleGroupAggregateFunctions;

	/** Stores aggregate values derived from base tuples and aggregate functions. */
	private TupleSet aggregateTuples;

	/** The aggregate attribute */
	private List<jol.lang.plan.Aggregate> aggregates;

	private Predicate predicate;

	/** The primary key. */
	protected Index primary;

	/** The secondary indices. */
	protected Map<Key, Index> secondary;

	private enum TupleKey {GROUP, AGGREGATE};
	private List<TupleKey> tupleKey;

	/**
	 * Create a new Aggregation table.
	 * @param context The runtime context.
	 * @param predicate The predicate containing the GroupBy/Aggregation
	 * @param type The type of aggregation.
	 * @throws PlannerException
	 */
	public Aggregation(Runtime context, Predicate predicate, Table.Type type, Class[] schemaTypes)
	throws PlannerException {
		super(predicate.name(), type, null, schemaTypes);
		this.predicate = predicate;
		this.aggregateFunctions = null;
		this.singleGroupAggregateFunctions = null;
		this.aggregateTuples    = new TupleSet(name());
		this.aggregates         = new ArrayList<jol.lang.plan.Aggregate>();

		this.aggregateTuples.refCount(false);

		/*
		 * Determine the primary key, which is always the group by key.
		 * Also determine the tuple key, which is used to create a result
		 * tuple from 1. a tuple containing just the group by values and
		 * 2. the list of aggregate functions for the given group.
		 * Just go look at Aggregation#result().
		 */
		this.tupleKey = new ArrayList<TupleKey>();
		List<Integer> groupKey = new ArrayList<Integer>();
		for (int position = 0; position < predicate.arguments().size(); position++) {
			Expression arg = predicate.argument(position);
			if (arg instanceof jol.lang.plan.Aggregate) {
				this.aggregates.add((jol.lang.plan.Aggregate)arg);
				this.tupleKey.add(TupleKey.AGGREGATE);
			}
			else if (!(arg instanceof jol.lang.plan.AggregateVariable)) {
				groupKey.add(position); // Group by key
				this.tupleKey.add(TupleKey.GROUP);
			}
		}
		super.key = new Key(groupKey);


		if (key().size() > 0) {
			this.aggregateFunctions = new HashMap<Tuple, List<Aggregate<C>>>();
		}
		else {
			this.singleGroupAggregateFunctions = groupGenerate();
		}


		if (type == Table.Type.TABLE) {
			this.primary = new HashIndex(context, this, key, Index.Type.PRIMARY);
			this.secondary = new HashMap<Key, Index>();
		}
	}

	public List<jol.lang.plan.Aggregate> aggregates() {
		return this.aggregates;
	}

	private List<Aggregate<C>> groupGenerate() throws PlannerException {
		List<Aggregate<C>> functions = new ArrayList<Aggregate<C>>();
		for (jol.lang.plan.Aggregate aggregate : this.aggregates) {
			functions.add(Aggregate.function(aggregate, predicate.schema()));
		}
		return functions;
	}

	/**
	 * Returns the (base) set of tuples contained in this table.
	 * @return The set of tuples that exist in this table after
	 * all insert/delete calls.
	 */
	private TupleSet result() {
		TupleSet result = new TupleSet(name());
		if (this.singleGroupAggregateFunctions != null) {
		    Object[] arry = new Object[this.singleGroupAggregateFunctions.size()];
		    for(int i = 0; i < arry.length; i++) {
		        arry[i] = this.singleGroupAggregateFunctions.get(i).result();
		    }
		    result.add(new Tuple(arry));
		} else {
			for (Tuple group : this.aggregateFunctions.keySet()) {
				Tuple tuple = result(group.clone());
				result.add(tuple);
			}
		}
		return result;
	}

	private Tuple result(Tuple group) {
		if (this.aggregateFunctions.containsKey(group)) {
			Iterator<Aggregate<C>> aValues = this.aggregateFunctions.get(group).iterator();
			Iterator<Object>      gbValues = group.iterator();
			Object arry[] = new Object[this.tupleKey.size()];
			for (int i = 0; i < arry.length; i++) {
			    switch(this.tupleKey.get(i)) {
			    case AGGREGATE:
			        arry[i] = aValues.next().result();
			        break;
			    case GROUP:
			        arry[i] = gbValues.next();
			        break;
			    }
			}
            return new Tuple(arry);
		}
		return null;
	}

	@Override
	public Iterable<Tuple> tuples() {
		return this.aggregateTuples.clone();
	}

	@Override
	/**
	 * The semantics of this method is somewhat different than that of
	 * regular tables. Insertions are applied as usual. However, deletions
	 * are applied during this operation, which may generate new insertions that
	 * become part of the delta set.
	 */
	public TupleSet insert(TupleSet insertions, TupleSet deletions) throws UpdateException {
		TupleSet expirations = new TupleSet();
		if (deletions != null && deletions.size() > 0) {
			TupleSet intersection = deletions.clone();
			intersection.retainAll(insertions);

			insertions.removeAll(intersection);
			deletions.removeAll(intersection);

			TupleSet delta = delete(deletions, expirations);
			deletions.clear();
			deletions.addAll(delta);
		}

		for (Tuple tuple : insertions) {
			List<Aggregate<C>> functions = null;
			if (this.singleGroupAggregateFunctions != null) {
				functions = this.singleGroupAggregateFunctions;
			}
			else {
				Tuple group = key().project(tuple);
				if (!this.aggregateFunctions.containsKey(group)) {
					try {
						this.aggregateFunctions.put(group, groupGenerate());
					} catch (PlannerException e) {
						throw new UpdateException(e.toString());
					}
				}
				functions = this.aggregateFunctions.get(group);
			}

			try {
				for (Aggregate<C> func : functions) {
					tuple = tuple.clone();
					func.insert(tuple); // perform this aggregate
				}
			} catch (JolRuntimeException e) {
				e.printStackTrace();
				System.exit(0);
			}
		}

		TupleSet delta = result();
		delta.removeAll(this.aggregateTuples);  // Only those newly inserted tuples remain.
		if (deletions != null) {
			delta.removeAll(deletions);
		}

		if (type() == Table.Type.EVENT) {
			if (this.singleGroupAggregateFunctions != null) {
				try {
					this.singleGroupAggregateFunctions = groupGenerate();
				} catch (PlannerException e) {
					throw new UpdateException(e.toString());
				}
			}
			else {
				this.aggregateFunctions.clear();
			}
			this.aggregateTuples.clear();
			return delta;
		}
		insertions = super.insert(delta, deletions);
		insertions.addAll(expirations);
		return insertions;
	}

	@Override
	public boolean insert(Tuple tuple) throws UpdateException {
		return this.aggregateTuples.add(tuple.clone());
	}

	/** Should only be called from within this Class.  */
	public TupleSet delete(Iterable<Tuple> deletions, TupleSet expire) throws UpdateException {
		if (type() == Table.Type.EVENT) {
			throw new UpdateException("Aggregation table " + name() + " is an event table!");
		}

		try {
			for (Tuple tuple : deletions) {
				if (this.singleGroupAggregateFunctions != null) {
					for (Aggregate<C> function : this.singleGroupAggregateFunctions) {
						function.delete(tuple);
						if (function.size() == 0) {
							expire.addAll(result());
							try {
								this.singleGroupAggregateFunctions = groupGenerate();
							} catch (PlannerException e) {
								throw new UpdateException(e.toString());
							}
							break; // we're out of tuples for this group.
						}
					}
				}
				else {
					Tuple group = key().project(tuple);
					if (this.aggregateFunctions.containsKey(group)) {
						List<Aggregate<C>> functions = this.aggregateFunctions.get(group);
						for (Aggregate<C> function : functions) {
							function.delete(tuple);
							if (function.size() == 0) {
								expire.add(result(group.clone()));
								this.aggregateFunctions.remove(group);
								break; // we're out of tuples for this group.
							}
						}
					}
				}
			}
		} catch (JolRuntimeException e) {
			e.printStackTrace();
		}

		TupleSet delta = new TupleSet(name());
		delta.addAll(tuples());

		delta.removeAll(result());  // removed = tuples that don't exist in after.
		return super.delete(delta); // signal indices that we've removed these tuples.
	}

	@Override
	public boolean delete(Tuple tuple) throws UpdateException {
		return this.aggregateTuples.remove(tuple);
	}


	@Override
	public Long cardinality() {
		return (long) this.aggregateTuples.size();
	}

	@Override
	public Index primary() {
		return this.primary;
	}

	@Override
	public Map<Key, Index> secondary() {
		return this.secondary;
	}
}
