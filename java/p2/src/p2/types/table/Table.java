package p2.types.table;

import java.util.*;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.UpdateException;
import p2.types.table.Index.IndexTable;
import xtc.util.SymbolTable;

public abstract class Table implements Iterable<Tuple>, Comparable {
	public static abstract class Callback {
		private static long ids = 0L;
		
		private Long id = new Long(ids++);
		public int hashCode() { return id.hashCode(); }
		
		public abstract void insertion(TupleSet tuples);
		public abstract void deletion(TupleSet tuples);
	}
	
	public static final Integer INFINITY = Integer.MAX_VALUE;

	public static class Catalog extends ObjectTable {
		private static final Key PRIMARY_KEY = new Key(0);

		public enum Field {TABLENAME, SIZE, LIFETIME, KEY, TYPES, OBJECT};
		private static final Class[] SCHEMA = { 
			String.class,    // The tablename
			Integer.class,   // The table size
			Float.class,     // The lifetime
			Key.class,       // The primary key
			TypeList.class,  // The type of each attribute
			Table.class      // The table object
		};
		
		public Catalog() {
			super("catalog", PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Table table = (Table) tuple.value(Field.OBJECT.ordinal());
			if (table == null) {
				String   name     = (String) tuple.value(Field.TABLENAME.ordinal());
				Integer  size     = (Integer) tuple.value(Field.SIZE.ordinal());
				Float    lifetime = (Float) tuple.value(Field.LIFETIME.ordinal());
				Key      key      = (Key) tuple.value(Field.KEY.ordinal());
				TypeList types    = (TypeList) tuple.value(Field.TYPES.ordinal());
				
				if (size.intValue() == INFINITY.intValue() && 
						lifetime.intValue() == INFINITY.intValue()) {
					table = new RefTable(name, key, types);
				}
				else {
					table = new BasicTable(name, size, lifetime, key, types);
				}
			}
			
			assert (table != null);
			tuple.value(Field.OBJECT.ordinal(), table);
			
			return super.insert(tuple);
		}
		
	}

	public static IndexTable index = null;
	public static Catalog catalog  = null;
	
	/* The table name. */
	protected String name;
	
	/* The number of attributes in this table. */
	protected TypeList attributeTypes;
	
	/* The maximum size of this table. */
	protected Integer size;
	
	/* the lifetime of a tuple, in seconds. */
	protected Float lifetime;
	
	/* The primary key. */
	protected Key key;
	
	protected TupleSet tuples;
	
	protected Index primary;
	
	protected Hashtable<Key, Index> secondary;
	
	private final Set<Callback> callbacks;
	
	protected Table(String name, Integer size, Float lifetime, Key key, TypeList attributeTypes) {
		this.name = name;
		this.attributeTypes = attributeTypes;
		this.size = size;
		this.lifetime = lifetime;
		this.key = key;
		this.tuples = new TupleSet(name);
		this.callbacks = new HashSet<Callback>();
		this.primary = new HashIndex(this, key, Index.Type.PRIMARY);
		this.secondary = new Hashtable<Key, Index>();
		
		if (catalog != null) {
			Table.register(name, size, lifetime, key, attributeTypes, this);
		}
		else {
			catalog = (Catalog) this;
			index   = new IndexTable();
		}
	}
	
	public final static void initialize() {
		new Catalog();
	}
	
	public final static Catalog catalog() {
		return Table.catalog;
	}
	
	public final static IndexTable index() {
		return Table.index;
	}
	
	public String toString() {
		String value = name.toString() + ", " + attributeTypes.toString() + 
		        ", " + size + ", " + lifetime + ", keys(" + key + "), {" +
		        attributeTypes.toString() + "}";
		for (Tuple t : this) {
			value += t.toString() + "\n";
		}
		return value;
	}
	
	/**
	 * Register a new callback on table updates.
	 * @param callback
	 */
	public void register(Callback callback) {
		this.callbacks.add(callback);
	}
	
	public void unregister(Callback callback) {
		this.callbacks.remove(callback);
	}
	
	public abstract boolean isEvent();
	
	public Iterator<Tuple> iterator() {
		return this.tuples.iterator();
	}
	
	private static void register(String name, Integer size, Float lifetime, 
			                     Key key, TypeList types, Table object) { 
		Tuple tuple = new Tuple(catalog.name(), name, size, lifetime, key, types, object);
		try {
			catalog.force(tuple);
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(0);
		}
	}
	
	public static boolean drop(String name) throws UpdateException {
		try {
			TupleSet tuples = catalog.primary().lookup(catalog.key().value(name));
			for (Tuple table : tuples) {
				return catalog.remove(table);
			}
		} catch (BadKeyException e) {
			// TODO Fatal error
			e.printStackTrace();
		}
		return false;
	}
	
	public static Table table(String name) {
		try {
			TupleSet table = catalog.primary().lookup(catalog.key().value(name));
			if (table.size() == 1) {
				return (Table) table.iterator().next().value(Catalog.Field.OBJECT.ordinal());
			}
			else if (table.size() > 1) {
				// TODO Fatal error.
			}
		} catch (BadKeyException e) {
			// TODO Fatal error.
			e.printStackTrace();
		}
		return null;
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
	public Class[] types() {
		return (Class[]) attributeTypes.toArray(new Class[attributeTypes.size()]);
	}
	
	/**
	 * @return The maximum size of this table. */
	public Integer size() {
		return this.size;
	}
	
	/**
	 * @return The maximum lifetime of a tuple in seconds. */
	public Number lifetime() {
		return this.lifetime;
	}
	
	/**
	 * @return The attribute position(s) that make up the primary key. 
	 */
	public Key key() {
		return this.key;
	}
	
	/**
	 * @return The primary index.
	 */
	public final Index primary() {
		return this.primary;
	}
	
	/**
	 * @return All defined secondary indices.
	 */
	public final Hashtable<Key, Index> secondary() {
		return this.secondary;
	}
	
	/**
	 * Get all current tuples that have a primary key
	 * conflict with the tuple set passed argument.
	 * @param tuples Tuples to test for conflicts
	 * @return All tuples in the table that conflict.
	 */
	public final TupleSet conflict(TupleSet tuples) {
		TupleSet conflicts = new TupleSet(name().toString());
		for (Tuple t : tuples) {
			TupleSet conflict = primary().lookup(t);
			if (conflict != null) {
				conflicts.addAll(conflict);
			}
		}
		return conflicts;
	}
	
	/**
	 * Insert a tuple into this table. 
	 * @return The delta set of tuples.
	 * @throws UpdateException 
	 **/
	public final TupleSet insert(TupleSet tuples) throws UpdateException {
		TupleSet delta = new TupleSet(name().toString());
		for (Tuple t : tuples) {
			t = t.clone();
			if (insert(t)) {
				delta.add(t);
			}
		}
		
		for (Callback callback : this.callbacks) {
			callback.insertion(delta);
		}
		
		this.tuples.addAll(delta);
		return delta;
	}
	
	/**
	 * Remove this tuple from the table. 
	 * Note: This will only schedule the removal of committed tuples.
	 * @throws UpdateException 
	 **/
	public final TupleSet remove(TupleSet tuples) throws UpdateException {
		TupleSet delta = new TupleSet(name().toString());
		for (Tuple t : tuples) {
			if (remove(t)) {
				delta.add(t);
			}
		}
		this.tuples.removeAll(delta);
		
		for (Callback callback : this.callbacks) {
			callback.deletion(delta);
		}
		
		return delta;
	}
	
	public final void force(Tuple tuple) throws UpdateException {
		TupleSet conflicts = primary().lookup(tuple);
		TupleSet forced = new TupleSet(tuple.name());
		forced.add(tuple);
		insert(forced);
		remove(conflicts);
	}
	
	/**
	 * Signal to table below to actually perform the insertion.
	 * @param t Tuple to be inserted.
	 * @return true if insertion occurred, false otherwise.
	 * @exception UpdateException Any problems that occurred.
	 */
	protected abstract boolean insert(Tuple t) throws UpdateException;
	
	/**
	 * Signal to table below to actually perform the deletion.
	 * @param t Tuple to be deleted.
	 * @throws UpdateException 
	 */
	protected abstract boolean remove(Tuple t) throws UpdateException;
}
