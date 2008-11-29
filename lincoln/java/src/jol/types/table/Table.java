package jol.types.table;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;
import jol.types.operator.Operator.OperatorTable;
import jol.types.table.Index.IndexTable;

/**
 * The interface that all table types must extend.
 * 
 * A table is responsible for the storage of tuples
 * with a specific schema. Tables provide insertion
 * and deletion methods that are called by the
 * query processor. Insertion and deletion calls
 * store or remove the passed in tuples and return
 * those tuples that actually modified the state of 
 * the table as delta sets.
 */
public abstract class Table implements Comparable<Table> {
	/** The name of the global scope. */
	public static final String  GLOBALSCOPE = "global";
	
	/**
	 * Listeners can be established on tables that are
	 * called whenever a set of tuples has been inserted
	 * or deleted. The most common client of the callback
	 * interface is the index structures that assist with
	 * table lookups.
	 */
	public static abstract class Callback {
		/** Callback identifier. */
		private static long ids = 0L;
		
		private Long id = new Long(ids++);
		@Override
		public int hashCode() { return id.hashCode(); }
		
		/** The set of tuples that were inserted into the table. */
		public abstract void insertion(TupleSet tuples);
		
		/** The set of tuples that were deleted from the table. */
		public abstract void deletion(TupleSet tuples);
	}
	

	/**
	 * The system catalog.
	 * This is a special table that stores references to all
	 * other tables (including itself) created by the runtime
	 * and the various user programs. 
	 */
	public static class Catalog extends ObjectTable {
		/** The runtime context. */
		private Runtime context;
		
		/** The index table. */
		private IndexTable index;
		
		/** The primary key: The name of the table. */
		private static final Key PRIMARY_KEY = new Key(0);

		/** The fields that make up a table tuple. */
		public enum Field {TABLENAME, TYPE, KEY, TYPES, OBJECT};
		
		/** The type of each field in a table tuple. */
		private static final Class[] SCHEMA = { 
			TableName.class, // Name
			String.class,    // Table type
			Key.class,       // The primary key
			TypeList.class,  // The type of each attribute
			Table.class      // The table object
		};
		
		/**
		 * Create a new catalog.
		 * @param context The runtime context.
		 */
		public Catalog(Runtime context) {
			super(context, new TableName(GLOBALSCOPE, "catalog"), PRIMARY_KEY, new TypeList(SCHEMA));
			this.context = context;
			this.index = null;
		}
		
		@Override
		protected boolean insert(Tuple tuple) throws UpdateException {
			Table table = (Table) tuple.value(Field.OBJECT.ordinal());
			if (table == null) {
				TableName name    = (TableName) tuple.value(Field.TABLENAME.ordinal());
				String   type     = (String) tuple.value(Field.TYPE.ordinal());
				Key      key      = (Key) tuple.value(Field.KEY.ordinal());
				TypeList types    = (TypeList) tuple.value(Field.TYPES.ordinal());

				if (type.equals(Type.TABLE.toString())) {
					table = new BasicTable(context, name, key, types);
				}
				else {
					throw new UpdateException("Don't know how to create table type " + type);
				}
			}
			
			tuple.value(Field.OBJECT.ordinal(), table);
			return super.insert(tuple);
		}
		
		/**
		 * The index table.
		 * @return The index table.
		 */
		IndexTable index() {
			return this.index;
		}
		
		/**
		 * Set the index table.
		 * @param index The index table.
		 */
		void index(IndexTable index) {
			this.index = index;
		}
		
		/**
		 * Drop the table with the given name.
		 * @param name The name of the table.
		 * @return true if table was dropped, false otherwise.
		 * @throws UpdateException
		 */
		public boolean drop(TableName name) throws UpdateException {
			try {
				TupleSet tuples = primary().lookupByKey(name);
				return delete(tuples).size() > 0;
					
			} catch (BadKeyException e) {
				e.printStackTrace();
				System.exit(0);
			}
			return false;
		}

		/**
		 * Get the table object with the given name.
		 * @param name The name of the table.
		 * @return The table object or null if !exist.
		 */
		public Table table(TableName name) {
			try {
				TupleSet table = primary().lookupByKey(name);
				
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
		
		/**
		 * Register the given table with the catalog.
		 * @param table The table to be registered.
		 * @return true if table registration succeeds, false otherwise.
		 */
		public boolean register(Table table) {
			Tuple tuple = new Tuple(table.name(), table.type().toString(), table.key(), new TypeList(table.types()), table);

			try {
				force(tuple);
				return true;
			} catch (UpdateException e) {
				e.printStackTrace();
			}
			return false;
		}
	}
	
	/** The table type enumeration. 
	 * TABLE: A materialized table (stores tuples)
	 * EVENT: Does not store the tuples inserted.
	 * FUNCTION: A table function. (no tuple storage)
	 * TIMER: Represents a wall-clock timer.
	 */
	public enum Type{TABLE, EVENT, FUNCTION, TIMER};
	
	/** The table type. */
	protected Type type;
	
	/** The table name. */
	protected TableName name;
	
	/** The type of each attribute in this table. */
	protected TypeList attributeTypes;
	
	/** The primary for this table. */
	protected Key key;
	
	/** The set of callbacks registered on this table. */
	protected final Set<Callback> callbacks;
	
	/**
	 * Constructs a new table object. (no catalog registration)
	 * @param name The name of the table.
	 * @param type The table type.
	 * @param key The primary key of this table. 
	 * @param attributeTypes The type of each attribute. 
	 */
	protected Table(TableName name, Type type, Key key, TypeList attributeTypes) {
		this.name = name;
		this.type = type;
		this.attributeTypes = attributeTypes;
		this.key = key;
		this.callbacks = new HashSet<Callback>();
	}
	
	/**
	 * Called to initialize a new Catalog under the given runtime context.
	 * @param context The runtime context.
	 * @return A new Catalog object.
	 */
	public final static Catalog initialize(Runtime context) {
		Catalog catalog = new Catalog(context);
		catalog.register(new IndexTable(context));
		catalog.register(catalog);
		catalog.register(new OperatorTable(context));
		return catalog;
	}
	
	
	@Override
	public String toString() {
		String value = name.toString() + ", " + attributeTypes.toString() + 
		               ", keys(" + key + "), {" + attributeTypes.toString() + "}";
		return value;
	}
	
	/**
	 * The number of tuples stored in this table.
	 * @return Number of stored tuples.
	 */
	public abstract Long cardinality();
	
	/**
	 * The table type.
	 * @return The table type.
	 */
	public Type type() {
		return this.type;
	}
	
	/**
	 * Register a new callback on table updates.
	 * @param callback The callback to register.
	 */
	public void register(Callback callback) {
		this.callbacks.add(callback);
	}
	
	/**
	 * Unregister a the given callback.
	 * @param callback The callback to deallocate.
	 */
	public void unregister(Callback callback) {
		this.callbacks.remove(callback);
	}
	
	
	/**
	 * Compare this table name to the other.
	 */
	public int compareTo(Table o) {
		return name().compareTo(o.name());
	}

	
	/**
	 * Get the table name.
	 * @return The name of this table. */
	public TableName name() {
		return this.name;
	}
	
	/**
	 * Get the type of each attribute (in schema order). 
	 * @return The number of attributes associated with this table. */
	public Class[] types() {
		return (Class[]) attributeTypes.toArray(new Class[attributeTypes.size()]);
	}
	
	/**
	 * The set of stored tuples.
	 * @return A reference to the set of stored tuples.
	 */
	public abstract Iterable<Tuple> tuples();
	
	/**
	 * The primary key.
	 * @return The primary key object.
	 */
	public Key key() {
		return this.key;
	}
	
	/**
	 * The primary index.
	 * @return The primary index.
	 */
	public abstract Index primary();
	
	/**
	 * A map containing all defined secondary indices.
	 * @return All defined secondary indices.
	 */
	public abstract Map<Key, Index> secondary();

	/**
	 * Force insert the given tuple. This method is
	 * provided as a way to insure the insertion of a tuple.
	 * The routine sidesteps the delta Datalog semantics.
	 * @param tuple The tuple to force insert.
	 * @throws UpdateException Bad tuple.
	 */
	public void force(Tuple tuple) throws UpdateException {
		TupleSet insertion = new TupleSet(name(), tuple);
		TupleSet conflicts = new TupleSet(name());
		insert(insertion, conflicts);
		delete(conflicts);
	}
	
	/**
	 * Insert a set of tuples into this table. Primary key conflicts
	 * are not removed from the table but rather returned in the
	 * the @param conflicts argument.
	 * @param tuples The tuples to be inserted.
	 * @param conflicts Any primary key conflicts that arise.
	 * @return The delta set of tuples.
	 * @throws UpdateException 
	 **/
	public TupleSet insert(TupleSet tuples, TupleSet conflicts) throws UpdateException {
		TupleSet delta = new TupleSet(name());
		for (Tuple t : tuples) {
			t = t.clone();
			
			Set<Tuple> oldvals = null;
			try {
				if (conflicts != null && primary() != null) {
					oldvals = primary().lookupByKey(primary().key().project(t));
			 	}
			} catch (BadKeyException e) {
				throw new UpdateException("couldn't insert", e);
			}

			t.refCount(1L); // TODO code review these semantics!
			if (insert(t)) {
				delta.add(t);

				if (conflicts != null && primary() != null) {
					conflicts.addAll(oldvals);
				}
				
				/* Update the indices here so that primary key
				 * conflicts from within the tupleset will show up
				 * during this method call.  */
				TupleSet insertion = new TupleSet(name());
				insertion.add(t);
				for (Callback callback : this.callbacks) {
					callback.insertion(insertion);
				}
			}
		}
		
		return delta;
	}
	
	/**
	 * Remove the set of tuples from the table. 
	 * @return The delta set of the delete tuples 
	 * (a.k.a. the tuples that were actually deleted by this operation.)
	 * @throws UpdateException 
	 **/
	public TupleSet delete(Iterable<Tuple> tuples) throws UpdateException {
		TupleSet delta = new TupleSet(name());
		
		for (Tuple t : tuples) {
			if (delete(t)) {
				delta.add(t);
			}
		}
		
		if (delta.size() > 0) {
			for (Callback callback : this.callbacks) {
				callback.deletion(delta);
			}
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
	 * @return true if the deletion occurred, false otherwise.
	 * @throws UpdateException 
	 */
	protected abstract boolean delete(Tuple t) throws UpdateException;
}
