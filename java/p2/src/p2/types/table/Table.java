package p2.types.table;

import java.util.*;

import p2.lang.plan.Aggregate;
import p2.lang.plan.Variable;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.P2RuntimeException;
import p2.types.exception.UpdateException;
import p2.types.table.Index.IndexTable;
import xtc.util.SymbolTable;

public abstract class Table implements Comparable<Table> {
	public static final Integer INFINITY    = Integer.MAX_VALUE;
	public static final String  GLOBALSCOPE = "global";
	
	public static abstract class Callback {
		private static long ids = 0L;
		
		private Long id = new Long(ids++);
		public int hashCode() { return id.hashCode(); }
		
		public abstract void insertion(TupleSet tuples);
		public abstract void deletion(TupleSet tuples);
	}
	

	public static class Catalog extends ObjectTable {
		private static final Key PRIMARY_KEY = new Key(0);

		public enum Field {TABLENAME, TYPE, SIZE, LIFETIME, KEY, TYPES, OBJECT};
		private static final Class[] SCHEMA = { 
			TableName.class, // Name
			String.class,    // Table type
			Integer.class,   // The table size
			Float.class,     // The lifetime
			Key.class,       // The primary key
			TypeList.class,  // The type of each attribute
			Table.class      // The table object
		};
		
		public Catalog() {
			super(new TableName(GLOBALSCOPE, "catalog"), PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Table table = (Table) tuple.value(Field.OBJECT.ordinal());
			if (table == null) {
				TableName name    = (TableName) tuple.value(Field.TABLENAME.ordinal());
				String   type     = (String) tuple.value(Field.TYPE.ordinal());
				Integer  size     = (Integer) tuple.value(Field.SIZE.ordinal());
				Float    lifetime = (Float) tuple.value(Field.LIFETIME.ordinal());
				Key      key      = (Key) tuple.value(Field.KEY.ordinal());
				TypeList types    = (TypeList) tuple.value(Field.TYPES.ordinal());

				if (type.equals(Type.TABLE.toString())) {
					if (size.intValue() == INFINITY.intValue() && 
							lifetime.intValue() == INFINITY.intValue()) {
						table = new RefTable(name, key, types);
					}
					else {
						table = new BasicTable(name, size, lifetime, key, types);
					}
				}
				else {
					throw new UpdateException("Don't know how to create table type " + type);
				}
			}
			
			assert (table != null);
			tuple.value(Field.OBJECT.ordinal(), table);
			
			return super.insert(tuple);
		}
		
	}
	
	public static IndexTable index = null;
	public static Catalog catalog  = null;
	
	public enum Event{NONE, INSERT, DELETE};
	
	public enum Type{TABLE, EVENT, FUNCTION};
	
	protected Type type;
	
	/* The table name. */
	protected TableName name;
	
	/* The number of attributes in this table. */
	protected TypeList attributeTypes;
	
	/* The maximum size of this table. */
	protected Integer size;
	
	/* the lifetime of a tuple, in seconds. */
	protected Float lifetime;
	
	protected Key key;
	
	protected final Set<Callback> callbacks;
	
	protected Table(TableName name, Type type, Integer size, Float lifetime, Key key, TypeList attributeTypes) {
		this.name = name;
		this.type = type;
		this.attributeTypes = attributeTypes;
		this.size = size;
		this.lifetime = lifetime;
		this.key = key;
		this.callbacks = new HashSet<Callback>();
		
		if (catalog != null) {
			Table.register(name, type, size, lifetime, key, attributeTypes, this);
		}
	}
	
	@Override
	public String toString() {
		String value = name.toString() + ", " + attributeTypes.toString() + 
		        ", " + size + ", " + lifetime + ", keys(" + key + "), {" +
		        attributeTypes.toString() + "}";
		if (tuples() != null) {
		for (Tuple t : tuples()) {
			value += t.toString() + "\n";
		}
		}
		return value;
	}
	
	public abstract Integer cardinality();
	
	public Type type() {
		return this.type;
	}
	
	public final static void initialize() {
		catalog = new Catalog();
		index   = new IndexTable();
	}
	
	public final static Catalog catalog() {
		return Table.catalog;
	}
	
	public final static IndexTable index() {
		return Table.index;
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
	
	private static void register(TableName name, Type type, 
								 Integer size, Float lifetime, 
			                     Key key, TypeList types, Table object) { 
		Tuple tuple = new Tuple(name, type.toString(), size, lifetime, key, types, object);
		try {
			catalog.force(tuple);
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(0);
		}
	}
	
	public static boolean drop(TableName name) throws UpdateException {
		try {
			TupleSet tuples = catalog.primary().lookup(name);
			return catalog.delete(tuples).size() > 0;
		} catch (BadKeyException e) {
			// TODO Fatal error
			e.printStackTrace();
		}
		return false;
	}
	
	public static Table table(TableName name) {
		try {
			TupleSet table = catalog.primary().lookup(name);
			if (table == null) {
				return null;
			}
			else if (table.size() == 1) {
				return (Table) table.iterator().next().value(Catalog.Field.OBJECT.ordinal());
			}
			else if (table.size() > 1) {
				System.err.println("More than one " + name + " table defined!");
				System.exit(1);
			}
		} catch (BadKeyException e) {
			// TODO Fatal error.
			e.printStackTrace();
		}
		return null;
	}
	

	public int compareTo(Table o) {
		return name().compareTo(o.name());
	}

	
	/**
	 * @return The name of this table. */
	public TableName name() {
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
	
	
	public abstract TupleSet tuples();
	
	public Key key() {
		return this.key;
	}
	
	/**
	 * @return The primary index.
	 */
	public abstract Index primary();
	
	/**
	 * @return All defined secondary indices.
	 */
	public abstract Hashtable<Key, Index> secondary();

	public void force(Tuple tuple) throws UpdateException {
		TupleSet insertion = new TupleSet(name());
		insertion.add(tuple);
		TupleSet conflicts = new TupleSet(name());
		insert(insertion, conflicts);
		delete(conflicts);
	}
	
	/**
	 * Insert a tuple into this table. 
	 * @return The delta set of tuples.
	 * @throws UpdateException 
	 **/
	public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
		TupleSet delta = new TupleSet(name());
		for (Tuple t : tuples) {
			t = t.clone();
			if (insert(t)) {
				delta.add(t);
				if (conflicts != null && primary() != null) {
					try {
						conflicts.addAll(primary().lookup(t));
					} catch (BadKeyException e) {
						throw new UpdateException(e.toString());
					}
				}
			}
		}
		
		
		for (Callback callback : this.callbacks) {
			callback.insertion(delta);
		}
		
		return delta;
	}
	
	/**
	 * Remove this tuple from the table. 
	 * Note: This will only schedule the removal of committed tuples.
	 * @throws UpdateException 
	 **/
	public TupleSet delete(TupleSet tuples) throws UpdateException {
		TupleSet delta = new TupleSet(name());
		for (Tuple t : tuples) {
			t = t.clone();
			if (delete(t)) {
				delta.add(t);
			}
		}
		
		for (Callback callback : this.callbacks) {
			callback.deletion(delta);
		}
		
		return delta;
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
	protected abstract boolean delete(Tuple t) throws UpdateException;
}
