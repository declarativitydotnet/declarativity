package jol.types.table;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import jol.core.Runtime;
import jol.lang.plan.Predicate;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.function.Aggregate;

/**
 * A table aggregation.
 * 
 *  Table aggregations maintain all tuples inserted into the table and
 *  remove those tuples on deletion. However, the tuples that are made
 *  visible to the outside world are the aggregate values generated 
 *  by set of resident tuples. A table aggregation is created if a
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
	
	/** The primary key. */
	protected Index primary;
	
	/** The secondary indices. */
	protected Map<Key, Index> secondary;
	
	/**
	 * Create a new Aggregation table.
	 * @param context The runtime context.
	 * @param predicate The predicate containing the GroupBy/Aggregation
	 * @param type The type of aggregation.
	 */
	public Aggregation(Runtime context, Predicate predicate, Table.Type type, Class[] schemaTypes) {
		super(predicate.name(), type, key(predicate), new TypeList(schemaTypes));
		this.aggregateFunctions = null;
		this.singleGroupAggregateFunctions = null;
		this.aggregateTuples    = new TupleSet(name());
		this.aggregates         = new ArrayList<jol.lang.plan.Aggregate>();
		
		this.aggregateTuples.refCount(false);
		
		for (jol.lang.plan.Expression arg : predicate) {
			if (arg instanceof jol.lang.plan.Aggregate) {
				this.aggregates.add((jol.lang.plan.Aggregate)arg);
			}
		}
		
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
	
	private List<Aggregate<C>> groupGenerate() {
		List<Aggregate<C>> functions = new ArrayList<Aggregate<C>>();
		for (jol.lang.plan.Aggregate aggregate : this.aggregates) {
			functions.add(Aggregate.function(aggregate));
		}
		return functions;
	}
	
	/**
	 * Determines the key based on the predicate.
	 * @param predicate The predicate defining the aggregate variable.
	 * @return A key the contains the GroupBy columns.
	 */
	private static Key key(Predicate predicate) {
		List<Integer> key = new ArrayList<Integer>();
		for (jol.lang.plan.Expression arg : predicate) {
			if (!(arg instanceof jol.lang.plan.AggregateVariable) &&
					!(arg instanceof jol.lang.plan.Aggregate)) {
				key.add(arg.position());
			}
		}
		return new Key(key);
	}
	
	/**
	 * Returns the (base) set of tuples contained in this table.
	 * @return The set of tuples that exist in this table after
	 * all insert/delete calls.
	 */
	private TupleSet result() {
		TupleSet result = new TupleSet(name());
		if (this.singleGroupAggregateFunctions != null) {
			Tuple tuple = new Tuple();
			for (Aggregate<C> aggregation : this.singleGroupAggregateFunctions) {
				tuple.append(null, aggregation.result());
			}
			result.add(tuple);
		} else {
			for (Tuple group : this.aggregateFunctions.keySet()) {
				Tuple tuple = result(group.clone());
				result.add(tuple);
			}
		}
		return result;
	}
	
	private Tuple result(Tuple group) {
		for (Aggregate<C> aggregation :  this.aggregateFunctions.get(group)) {
			group.insert(aggregation.position(), aggregation.result());
		}
		return group;
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
				Tuple key = key().project(tuple);
				if (!this.aggregateFunctions.containsKey(key)) {
					this.aggregateFunctions.put(key, groupGenerate());
				}
				functions = this.aggregateFunctions.get(key);
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
				this.singleGroupAggregateFunctions = groupGenerate();
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
							this.singleGroupAggregateFunctions = groupGenerate();
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
		return (long)this.aggregateTuples.size();
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
