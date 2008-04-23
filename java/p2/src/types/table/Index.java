package types.table;

import java.util.Iterator;
import java.util.Vector;

import types.basic.Tuple;
import types.basic.TupleSet;


public abstract class Index {
	public enum Type {PRIMARY, SECONDARY};
	
	public enum Method {HASH};
	
	private Type type;
	
	private Method method;
	
	/* The index key referencing attribute positions in the indexed table. */
	private Key key;
	
	public Index(Type type, Method method, Key key) {
		this.type = type;
		this.method = method;
		this.key = key;
	}
	
	public Type type() {
		return this.type;
	}
	
	public Method method() {
		return this.method;
	}
	
	public Key key() {
		return key;
	}
	
	/* The table over which this index is defined. */
	public abstract Table table();
	
	/* Uses the index key as the lookup key and
	 * the values from the given tuple. */
	public abstract TupleSet lookup(Tuple t);
	
	public abstract TupleSet lookup(Key.Value key);
	
	public abstract boolean containsKey(Key.Value key);
	
	/* Index the given tuples. */
	public abstract Tuple insert(Tuple t);
	
	/* Remove this tuple from the index. */
	public abstract void remove(Tuple t);
	
	/* Validate this index. */
	public abstract boolean commit();
	
	/* An iterator over all keys in the index. */
	public abstract Iterator<Key.Value> keys();
	
	/* An iterator over all tuple values. */
	public abstract Iterator<Tuple> tuples();
	
}
