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
	
	private Hashtable<Tuple,TupleSet> tuples;
	
	private Hashtable<Tuple, Aggregate> aggregates;
	
	private p2.lang.plan.Aggregate aggregate;
	
	protected Index primary;
	
	protected Hashtable<Key, Index> secondary;
	
	public Aggregation(Predicate predicate, Table.Type type) {
		super(predicate.name(), type, key(predicate), types(predicate));
		this.tuples = new Hashtable<Tuple, TupleSet>();
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
		TupleSet current = new TupleSet(name());
		if (aggregates != null) {
			for (Aggregate aggregate : aggregates.values()) {
				current.add(aggregate.result());
			}
		}
		return current;
	}
	
	public TupleSet insert(TupleSet insertions, TupleSet deletions) throws UpdateException {
		deletions.removeAll(insertions);
		
		TupleSet previous = tuples();
		
		/* Perform aggregation. */
		for (Tuple tuple : insertions) {
			insert(tuple);
		}
		
		if (deletions != null && deletions.size() > 0) {
			delete(deletions);
			deletions.clear();
		}
		
		TupleSet current = tuples();
		
		if (deletions != null) {
			deletions.addAll(previous);    // Add all previous tuples
			deletions.removeAll(current);  // Remove all current tuples
		}
		
		TupleSet delta = new TupleSet(name());
		delta.addAll(current); // Add all current tuples
		delta.removeAll(previous); // Remove those that existed before
		
		for (Callback callback : callbacks) {
			callback.insertion(delta);
			callback.deletion(deletions);
		}
		
		if (type() == Table.Type.EVENT) {
			this.tuples.clear();
			this.aggregates.clear();
		}
		return delta;
	}
	
	@Override
	public TupleSet delete(TupleSet deletions) throws UpdateException {
		TupleSet previous = tuples();
		
		/* Perform deletions. */
		for (Tuple tuple : deletions) {
			delete(tuple);
		}
		
		TupleSet current = tuples();
		
		TupleSet updates = new TupleSet(name());
		updates.addAll(previous);    // Add all previous tuples
		updates.removeAll(current);  // Remove all current tuples
		
		TupleSet delta = new TupleSet(name());
		delta.addAll(current);     // Add all current tuples
		delta.removeAll(previous); // Remove those that existed before
		
		for (Callback callback : callbacks) {
			callback.insertion(delta);
			callback.deletion(updates);
		}
		
		return delta;
	}
	
	@Override
	public boolean delete(Tuple tuple) throws UpdateException {
		Tuple key = key().project(tuple);
		TupleSet group = tuples.get(key);
		if (group == null) {
			return false;
		}
		group.remove(tuple);
		
		if (group.size() == 0) {
			tuples.remove(key);
			aggregates.remove(key);
			return true;
		}
		else {
			Aggregate function = aggregates.get(key);
			if (function.result().equals(tuple)) {
				/* Recompute aggregate of tuple group. */
				function.reset();
				for (Tuple t : group) {
					try {
						function.evaluate(t);
					} catch (P2RuntimeException e) {
						e.printStackTrace();
						System.exit(1);
					}
				}
				/* Did deletion caused an aggregate change? */
				return !function.result().equals(tuple);
			}
		}
		return false; // Aggregate remains unchanged.
	}

	@Override
	public boolean insert(Tuple tuple) throws UpdateException {
		Tuple key = key().project(tuple);
		if (!tuples.containsKey(key)) {
			tuples.put(key, new TupleSet(name()));
		}
		tuples.get(key).add(tuple);
		
		if (!aggregates.containsKey(key)) {
			aggregates.put(key, Aggregate.function(this.aggregate));
		}
		try {
			aggregates.get(key).evaluate(tuple);
		} catch (P2RuntimeException e) {
			throw new UpdateException(e.toString());
		}
		return true;
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
