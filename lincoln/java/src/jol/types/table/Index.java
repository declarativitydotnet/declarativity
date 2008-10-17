package jol.types.table;

import jol.core.Runtime;
import java.lang.reflect.Constructor;
import java.util.Iterator;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;

/**
 * The interface to an index structure.
 * 
 * An index is responible for looking up tuples by some
 * subset of tuple values identified by the index key {@link Key}.
 */
public abstract class Index implements Comparable<Index>, Iterable<Tuple> {
	
	/**
	 * A listener class that is used to registered with
	 * the table being indexed. This listener informs the
	 * index that insertions and deletions have occurred. The
	 * listener will perform index maintenance as tuples
	 * are inserted/deleted from the reference table.
	 */
	private class Listener extends Table.Callback {
		@Override
		public void deletion(TupleSet tuples) {
			for (Tuple tuple : tuples) {
				remove(tuple);
			}
		}
		@Override
		public void insertion(TupleSet tuples) {
			for (Tuple tuple : tuples) {
				insert(tuple);
			}
		}
	}
	
	/**
	 * The index table stores all indices defined in the Runtime.
	 */
	public static class IndexTable extends ObjectTable {
		/** The runtime context. */
		private Runtime context;
		
		/** The name of the index table. */
		private static final TableName TABLENAME = new TableName(GLOBALSCOPE, "index");
		
		/** The primary key: <Tablename, Key> */
		private static final Key PRIMARY_KEY = new Key(0,1); 
		
		/** The field names. */
		public enum Field {TABLENAME, KEY, TYPE, CLASSNAME, OBJECT};
		
		/** The field types. */
		private static final Class[] SCHEMA = { 
			TableName.class, // table name
			Key.class,       // key
			Type.class,      // the type
			String.class,    // the class type
			Index.class      // the object
		};
		
		/** 
		 * Create a new index table (should be one per runtime).
		 * @param context The runtime context.
		 */
		public IndexTable(Runtime context) {
			super(context, new TableName(GLOBALSCOPE, "index"), PRIMARY_KEY, new TypeList(SCHEMA));
			this.context = context;
		}
		
		@Override
		public boolean insert(Tuple tuple) throws UpdateException {
			if (tuple.value(Field.OBJECT.ordinal()) == null) {
				try {
					Class ctype = Class.forName((String)tuple.value(Field.CLASSNAME.ordinal()));
					Constructor constructor = ctype.getConstructor(Table.class, Key.class, Type.class);
					TableName name  = (TableName) tuple.value(Field.TABLENAME.ordinal());
					Table table = context.catalog().table(name);
					
					/* Create the index object. */
					Key key = (Key) tuple.value(Field.KEY.ordinal());
					Type type = (Type)tuple.value(Field.TYPE.ordinal());
					Index index = (Index) constructor.newInstance(table, key, type);
					tuple.value(Field.OBJECT.ordinal(), index);
				} catch (Exception e) {
					throw new UpdateException(e.toString());
				}
			}
			return super.insert(tuple);
		}

		@Override
		public boolean delete(Tuple tuple) throws UpdateException {
			return super.delete(tuple);
		}

	}
	
	/** The index type enumeration. */
	public enum Type{PRIMARY, SECONDARY};
	
	/** The table over which this index object is defined. */
	private Table table;
	
	/** The index key referencing attribute positions in the indexed table. */
	private Key key;
	
	/** The type of index. */
	private Type type;
	
	/**
	 * Create a new index. This constuctor will register the index
	 * with the IndexTable. The index will register a listener with
	 * the reference table.
	 * @param context The runtime context.
	 * @param table The table being indexed.
	 * @param key The key used to index the table.
	 * @param type The type of index being created.
	 */
	public Index(Runtime context, Table table, Key key, Type type) {
		this.table = table;
		this.key = key;
		this.type = type;
		if (context.catalog() != null) {
			try {
				IndexTable iTable = context.catalog().index();
				if (iTable != null) {
					iTable.insert(new Tuple(table.name(), key, type, this.getClass().getName(), this));
				}
			} catch (UpdateException e) {
				e.printStackTrace();
			}
		}
		this.table.register(new Listener());
	}
	
	@Override
	public abstract String toString();
	
	/**
	 * Compares this index to another index using
	 * the {@link Index#toString()} method.
	 */
	public int compareTo(Index i) {
		return toString().compareTo(i.toString());
	}
	
	/** 
	 * The table over which this index is defined. 
	 * @return The table object.
	 */
	public Table table() {
		return this.table;
	}
	
	/**
	 * The type of index.
	 * @return The index type.
	 */
	public Type type() {
		return this.type;
	}
	
	/**
	 * The index key.
	 * @return The key object.
	 */
	public Key key() {
		return key;
	}
	
	/** An iterator over all tuple values. */
	public abstract Iterator<Tuple> iterator();
	
	/** 
	 * Uses the index key as the lookup key and
	 * the values from the given tuple. 
	 * @param t A tuple to look up 
	 * (assumed to be a pre-projected key) 
	 * @return A set containing the lookup result.
	 */
	public abstract TupleSet lookupByKey(Tuple t) throws BadKeyException;
	
	/**
	 * Quick way to lookup a set of values. The arity of the
	 * values must match that of the index key.
	 * @param values A argument list of values that should be used
	 * to perform the lookup.
	 * @return A set containing the lookup result.
	 * @throws BadKeyException If the argument values do not match the key.
	 */
	public TupleSet lookupByKey(Comparable... keyValues) throws BadKeyException 
	{
		return lookupByKey(new Tuple(keyValues));
	}
	
	/** Index the given tuple. */
	protected abstract void insert(Tuple t);
	
	/** Remove the tuple from the index. */
	protected abstract void remove(Tuple t);

}
