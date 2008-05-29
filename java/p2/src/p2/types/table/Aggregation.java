package p2.types.table;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.function.Aggregate;

public class Aggregation extends Table {
	
	private TupleSet tuples;
	
	private Hashtable<Tuple, Aggregate> aggregates;
	
	private p2.lang.plan.Aggregate aggregate;
	
	protected Index primary;
	
	protected Hashtable<Key, Index> secondary;
	
	public Aggregation(Predicate predicate, Table.Type type) {
		super(predicate.name(), type, Integer.MAX_VALUE, Float.MAX_VALUE, key(predicate), types(predicate));
		this.tuples = new TupleSet(predicate.name());
		this.aggregates = new Hashtable<Tuple, Aggregate>();
		if (type == Table.Type.TABLE) {
			this.primary = new HashIndex(this, key, Index.Type.PRIMARY);
			this.secondary = new Hashtable<Key, Index>();
		}
		
		for (p2.lang.plan.Expression arg : predicate) {
			if (arg instanceof p2.lang.plan.Aggregate) {
				this.aggregate = (p2.lang.plan.Aggregate) arg;
				break;
			}
		}
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
		return this.tuples.clone();
	}
	
	public TupleSet insert(TupleSet insertions, TupleSet deletions) throws UpdateException {
		TupleSet previous = new TupleSet(name());
		for (Aggregate aggregate : aggregates.values()) {
			previous.add(aggregate.result());
		}
		
		/* Perform aggregation. */
		for (Tuple tuple : insertions) {
			insert(tuple);
		}
		
		if (deletions != null) {
			deletions.addAll(previous);        // Add all previous tuples
			deletions.removeAll(this.tuples);  // Remove all current tuples
		}
		
		TupleSet delta = new TupleSet(name());
		delta.addAll(this.tuples); // Add all current tuples
		delta.removeAll(previous); // Remove those that existed before
		
		for (Callback callback : callbacks) {
			callback.insertion(delta);
		}
		
		if (type() == Table.Type.EVENT) {
			this.tuples.clear();
			this.aggregates.clear();
		}
		return delta;
	}
	
	@Override
	public boolean delete(Tuple tuple) throws UpdateException {
		return this.tuples.remove(tuple);
	}

	@Override
	public boolean insert(Tuple tuple) throws UpdateException {
		Tuple key = key().project(tuple);
		
		if (!aggregates.containsKey(key)) {
			aggregates.put(key, Aggregate.function(this.aggregate));
		}
		try {
			aggregates.get(key).evaluate(tuple);
		} catch (P2RuntimeException e) {
			throw new UpdateException(e.toString());
		}
		return tuples.add(aggregates.get(key).result());
	}

	@Override
	public Integer cardinality() {
		return this.tuples.size();
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
