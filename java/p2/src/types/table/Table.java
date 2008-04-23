package types.table;

import java.util.*;

import types.basic.SimpleTupleSet;
import types.basic.Tuple;
import types.basic.TupleSet;
import types.exception.UpdateException;
import types.table.Key.Value;

public abstract class Table implements Iterable<Tuple>, Comparable {
	public enum Type{BASIC, REFCOUNT, OBJECT};
	
	/* The table name. */
	private String name;
	
	/* The number of attributes in this table. */
	private Schema schema;
	
	/* The maximum size of this table. */
	private Long size;
	
	/* the lifetime of a tuple, in seconds. */
	private Long lifetime;
	
	/* The primary key. */
	private Key key;
	
	/* A buffer holding delta insertions. */
	protected Hashtable<Key.Value, Tuple> insertDelta;
	
	/* A buffer holding delta deletions. */
	protected TupleSet deleteDelta;
	
	protected Table(String name, Schema schema, Long size, Long lifetime, Key key) {
		this.name = name;
		this.schema = schema;
		this.size = size;
		this.lifetime = lifetime;
		this.key = key;
		this.insertDelta = new Hashtable<Key.Value, Tuple>();
		this.deleteDelta = new SimpleTupleSet();
	}

	public int compareTo(Object o) {
		if (o instanceof Table) {
			return name().compareTo(((Table)o).name());
		}
		return hashCode() < o.hashCode() ? -1 : 1;
	}

	/**
	 * @return The name of this table. */
	public String name() {
		return this.name;
	}
	
	/**
	 * @return The number of attributes associated with this table. */
	public Schema schema() {
		return this.schema;
	}
	
	/**
	 * @return Field position for the given name. */
	public int schema(String name) {
		return schema().field(name);
	}
	
	/**
	 * @return The maximum size of this table. */
	public Long size() {
		return this.size;
	}
	
	/**
	 * @return The maximum lifetime of a tuple in seconds. */
	public Long lifetime() {
		return this.lifetime;
	}
	
	/**
	 * @return The attribute position(s) that make up the primary key. 
	 */
	public Key key() {
		return this.key;
	}
	
	/**
	 *  Insert a tuple into this table. 
	 *  @return The delta set of tuples.
	 **/
	public final TupleSet insert(TupleSet tuples) {
		TupleSet delta = new SimpleTupleSet();

		for (Tuple t : tuples) {
			Tuple previous = insertDelta.put(key.value(t), t);
			if (previous != null && !previous.equals(t)) {
				delta.add(t);
			}
		}

		return delta; // Return the delta of this tuple insertion.
	}
	
	/**
	 *  Remove this tuple from the table. 
	 *  Note: This will only schedule the removal of committed tuples.
	 **/
	public final void remove(TupleSet tuples) {
		for (Tuple t : tuples) {
			for (Tuple lookup : primary().lookup(t)) {
				if (t.frozen() && lookup.equals(t)) {
					deleteDelta.add(t);
				}
			}
		}
	}
	
	/**
	 * Called at the start of a new Datalog iteration.
	 * Resets all delta sets.
	 */
	public void begin() {
		this.insertDelta.clear();
		this.deleteDelta.clear();
	}
	
	/** 
	 * Commit all insertions and deletions. Basically ensure
	 * all integrity constrains (e.g., primary key) are met. 
	 * @return TupleSet All deleted tuples, either from implicit deletions 
	 * from primary key conflicts with inserted tuples.
	 **/
	public void commit() {
		for (Tuple t : deleteDelta) {
			try {
				if (!remove(t)) {
					deleteDelta.remove(t);
				}
			} catch (UpdateException e) {
				e.printStackTrace();
				deleteDelta.remove(t);
			}
		}
		
		/* Commit inserted tuples. */
		for (Tuple t : insertDelta.values()) {
			Tuple previous;
			try {
				previous = insert(t);
			    if (previous != null && previous.equals(t)) {
			    	insertDelta.remove(key.value(t));
			    }
			} catch (UpdateException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	
	public TupleSet deleteDelta() {
		return this.deleteDelta;
	}
	
	public TupleSet insertDelta() {
		return new SimpleTupleSet(this.insertDelta.values());
	}
	
	/**
	 * @return The primary index.
	 */
	public abstract Index primary();
	
	/**
	 * @return All defined secondary indices.
	 */
	public abstract Hashtable<Vector<Integer>, Index> secondary();
	
	/**
	 * Signal to table below to actually perform the insertion.
	 * @param t Tuple to be inserted.
	 * @return Tuple removed from primary key violation, null if none.
	 * @exception UpdateException Any problems that occurred.
	 */
	protected abstract Tuple insert(Tuple t) throws UpdateException;
	
	/**
	 * Signal to table below to actually perform the deletion.
	 * @param t Tuple to be deleted.
	 * @throws UpdateException 
	 */
	protected abstract boolean remove(Tuple t) throws UpdateException;
}
