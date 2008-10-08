package p2.types.table;

import java.util.*;

import p2.core.Runtime;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.UpdateException;
import p2.types.operator.Operator.OperatorTable;
import p2.types.table.Index.IndexTable;

public abstract class Table implements Comparable<Table> {
	public static final String  GLOBALSCOPE = "global";
	
	public static abstract class Callback {
		private static long ids = 0L;
		
		private Long id = new Long(ids++);
		public int hashCode() { return id.hashCode(); }
		
		public abstract void insertion(TupleSet tuples);
		public abstract void deletion(TupleSet tuples);
	}
	

	public static class Catalog extends ObjectTable {
		private Runtime context;
		
		private IndexTable index;
		
		private static final Key PRIMARY_KEY = new Key(0);

		public enum Field {TABLENAME, TYPE, KEY, TYPES, OBJECT};
		private static final Class[] SCHEMA = { 
			TableName.class, // Name
			String.class,    // Table type
			Key.class,       // The primary key
			TypeList.class,  // The type of each attribute
			Table.class      // The table object
		};
		
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
		
		public IndexTable index() {
			return this.index;
		}
		
		public void index(IndexTable index) {
			this.index = index;
		}
		
		public boolean drop(TableName name) throws UpdateException {
			try {
				TupleSet tuples = primary().lookup(name);
				return delete(tuples).size() > 0;
			} catch (BadKeyException e) {
				e.printStackTrace();
			}
			return false;
		}
		
		public Table table(TableName name) {
			try {
				TupleSet table = primary().lookup(name);
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
	
	public enum Event{NONE, INSERT, DELETE};
	
	public enum Type{TABLE, EVENT, FUNCTION};
	
	protected Type type;
	
	/* The table name. */
	protected TableName name;
	
	/* The number of attributes in this table. */
	protected TypeList attributeTypes;
	
	protected Key key;
	
	protected final Set<Callback> callbacks;
	
	protected Table(TableName name, Type type, Key key, TypeList attributeTypes) {
		this.name = name;
		this.type = type;
		this.attributeTypes = attributeTypes;
		this.key = key;
		this.callbacks = new HashSet<Callback>();
	}
	
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
	
	public abstract Integer cardinality();
	
	public Type type() {
		return this.type;
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
	 * @throws UpdateException 
	 */
	protected abstract boolean delete(Tuple t) throws UpdateException;
}
