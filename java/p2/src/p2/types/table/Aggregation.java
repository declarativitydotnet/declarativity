package p2.types.table;
import java.util.Hashtable;
import java.util.List;
import java.util.ArrayList;
import p2.lang.plan.Predicate;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.function.Aggregate;

public class Aggregation extends Table {
	
	/* Stores base tuples in aggregate functions. */
	private Hashtable<Tuple, Aggregate> baseTuples;
	
	/* Stores aggregate values derrived from base tuples and aggregate functions. */
	private TupleSet aggregateTuples;
	
	private p2.lang.plan.Aggregate aggregate;
	
	protected Index primary;
	
	protected Hashtable<Key, Index> secondary;
	
	public Aggregation(Predicate predicate, Table.Type type) {
		super(predicate.name(), type, key(predicate), types(predicate));
		this.baseTuples = new Hashtable<Tuple, Aggregate>();
		this.aggregateTuples = new TupleSet(name());
		
		for (p2.lang.plan.Expression arg : predicate) {
			if (arg instanceof p2.lang.plan.Aggregate) {
				this.aggregate = (p2.lang.plan.Aggregate) arg;
				break;
			}
		}
		
		if (type == Table.Type.TABLE) {
			this.primary = new HashIndex(this, key, Index.Type.PRIMARY);
			this.secondary = new Hashtable<Key, Index>();
		}
	}
	
	public p2.lang.plan.Aggregate variable() {
		return this.aggregate;
	}
	
	private static Key key(Predicate predicate) {
		List<Integer> key = new ArrayList<Integer>();
		for (p2.lang.plan.Expression arg : predicate) {
			if (!(arg instanceof p2.lang.plan.Aggregate)) {
				key.add(arg.position());
			}
		}
		return new Key(key);
	}
	
	private static TypeList types(Predicate predicate) {
		TypeList types = new TypeList();
		for (p2.lang.plan.Expression arg : predicate) {
			types.add(arg.type());
		}
		return types;
	}
	
	@Override
	public TupleSet tuples() {
		return this.aggregateTuples.clone();
	}
	
	private TupleSet values() {
		TupleSet values = new TupleSet(name());
		for (Aggregate value : baseTuples.values()) {
			values.add(value.result());
		}
		return values;
	}
	
	public TupleSet insert(TupleSet insertions, TupleSet deletions) throws UpdateException {
		if (deletions.size() > 0) {
			TupleSet intersection = deletions.clone();
			intersection.retainAll(insertions);
		
			insertions.removeAll(intersection);
			deletions.removeAll(intersection);
			TupleSet delta = delete(deletions);
			deletions.clear();
			deletions.addAll(delta);
		}
		

		
		
		for (Tuple tuple : insertions) {
			Tuple key = key().project(tuple);
			if (!baseTuples.containsKey(key)) {
				baseTuples.put(key, Aggregate.function(this.aggregate));
			}
			try {
				baseTuples.get(key).insert(tuple);
			} catch (P2RuntimeException e) {
				e.printStackTrace();
				System.exit(0);
			}
		}
		
		TupleSet delta = values();
		delta.removeAll(tuples());  // Only those newly inserted tuples remain.
		delta.removeAll(deletions);
		
		if (type() == Table.Type.EVENT) {
			this.baseTuples.clear();
			this.aggregateTuples.clear();
			return delta;
		}
		insertions = super.insert(delta, deletions);
		return insertions;
	}
	
	@Override
	public boolean insert(Tuple tuple) throws UpdateException {
		return this.aggregateTuples.add(tuple);
	}
	
	public TupleSet delete(TupleSet deletions) throws UpdateException {
		if (type() == Table.Type.EVENT) {
			throw new UpdateException("Aggregation table " + name() + " is an event table!");
		}
		
		for (Tuple tuple : deletions) {
			Tuple key = key().project(tuple);
			if (this.baseTuples.containsKey(key)) {
				try {
					this.baseTuples.get(key).delete(tuple);
					if (this.baseTuples.get(key).tuples().size() == 0) {
						this.baseTuples.remove(key);
					}
				} catch (P2RuntimeException e) {
					e.printStackTrace();
				}
			}
		}
		
		TupleSet delta = new TupleSet(name());
		delta.addAll(tuples());
		
		delta.removeAll(values());  // removed = tuples that don't exist in after.
		return super.delete(delta); // signal indices that we've removed these tuples.
	}
	
	@Override
	public boolean delete(Tuple tuple) throws UpdateException {
		return this.aggregateTuples.remove(tuple);
	}


	@Override
	public Integer cardinality() {
		return this.aggregateTuples.size();
	}

	@Override
	public Index primary() {
		return this.primary;
	}
	
	@Override
	public Hashtable<Key, Index> secondary() {
		return this.secondary;
	}

}
