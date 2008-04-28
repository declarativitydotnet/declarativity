package types.table;

import java.lang.reflect.Constructor;
import java.util.*;
import types.basic.SimpleTupleSet;
import types.basic.Tuple;
import types.basic.TupleSet;
import types.exception.BadKeyException;
import types.exception.UpdateException;
import types.table.Key.Value;
import xtc.util.SymbolTable;

public abstract class Table implements Iterable<Tuple>, Comparable {
	public static final Integer INFINITY = Integer.MAX_VALUE;

	public static class Name implements Comparable {
		public String name;
		public String object;

		public Name(String name, String object) {
			this.name = name;
			this.object = object;
		}

		public String toString() {
			return name();
		}
		
		public int compareTo(Object o) {
			return o instanceof Table.Name ? 
					name.compareTo(((Table.Name)o).name) : -1;
		}

		public String name() {
			return this.name;
		}

		public String object() {
			return this.object;
		}
	}
	
	private class ForeignKey {
		/* The foreign table. */
		private Table foreign;

		/* The fields of the local tuple that reference
		 * the remote primary key.  */
		private Key local;

		public ForeignKey(Table foreign, Key local) {
			assert(foreign.key().equals(local));
			this.foreign = foreign;
			this.local = local;
			this.foreign.remoteConstraint(this);
		}

		/**
		 * The foreign table will call this method
		 * when tuples from it have been removed.
		 * @param tuples The set of removed tuples.
		 */
		public void remove(TupleSet tuples) {

		}

		/**
		 * The reference table uses this to check
		 * for constraint violations.
		 * @param tuple Tuple to check.
		 * @return true if constraint is satisfied, false otherwise.
		 */
		public boolean check(Tuple tuple) {

			return true;
		}
	}


	public static class Catalog extends ObjectTable {

		protected Catalog(Name name, Schema schema, Integer size, Number lifetime, Key key) {
			super(name, schema, key);
		}

		private static final Key PRIMARY_KEY = new Key(0);

		private static final Schema SCHEMA = 
			new Schema(new Schema.Entry("Name",  Name.class),
					new Schema.Entry("Schema",   Schema.class),
					new Schema.Entry("Size",     Integer.class),
					new Schema.Entry("Lifetime", Integer.class),
					new Schema.Entry("Key",      Key.class),
					new Schema.Entry("Table",    Table.class));
		
		@Override
		protected Tuple insert(Tuple tuple) throws UpdateException {
			Table table = null;
			Name name = (Name) tuple.value(0);
			try {
				Class eclass = Class.forName(name.object);
				Constructor constructor = eclass.getConstructor(String.class, Schema.class, Integer.class, Number.class, Key.class);
				table = (Table) constructor.newInstance(name.object, 
						                                name.name,
						                                tuple.value(SCHEMA.field("Schema")),
						                                tuple.value(SCHEMA.field("Size")),  
						                                tuple.value(SCHEMA.field("Lifetime")), 
						                                tuple.value(SCHEMA.field("Key")));
			} catch (Exception e) {
				throw new UpdateException(e.toString());
			}
			assert (table != null);
			tuple.value(SCHEMA.field("Table"), table);
			
			return super.insert(tuple);
		}
		
	}

	private static Catalog CATALOG = null;
	
	/* The table name. */
	private Name name;
	
	/* The number of attributes in this table. */
	private Schema schema;
	
	/* The maximum size of this table. */
	private Integer size;
	
	/* the lifetime of a tuple, in seconds. */
	private Number lifetime;
	
	/* The primary key. */
	private Key key;
	
	private Set<ForeignKey> localConstraints;
	
	private Set<ForeignKey> remoteConstraints;
	
	
	protected Table(Name name, Schema schema, Integer size, Number lifetime, Key key) {
		this.name = name;
		this.schema = schema;
		this.size = size;
		this.lifetime = lifetime;
		this.key = key;
		this.localConstraints = new HashSet<ForeignKey>();
		this.remoteConstraints = new HashSet<ForeignKey>();
	}
	
	public static void initialize() {
		
	}
	
	public SymbolTable symbolTable() {
		return null;
	}
	
	public static Catalog catalog() {
		return CATALOG;
	}
	
	public static Table create(Name name, Schema schema, Integer size, Integer lifetime, Key key) 
	throws UpdateException {
		if (catalog() == null) {
			throw new UpdateException("Catalog has not been created!");
		}
		
		Tuple tuple = new Tuple(catalog().name().toString(), name, schema, size, lifetime, key);
		Tuple table = catalog().insert(tuple);
		if (table != null) {
			return (Table) table.value(schema.field("Table"));
		}
		return null;
	}
	
	public static Table create(Name name, Schema schema, Key key) 
	throws UpdateException {
		return create(name, schema, INFINITY, INFINITY, key);
	}
	
	public static boolean drop(String name) throws UpdateException {
		if (catalog() == null) {
			throw new UpdateException("Catalog has not been created!");
		}
		
		try {
			TupleSet tuples = catalog().primary().lookup(catalog().key().value(name));
			for (Tuple table : tuples) {
				return catalog().remove(table);
			}
		} catch (BadKeyException e) {
			// TODO Fatal error
			e.printStackTrace();
		}
		return false;
	}
	
	public static Table table(String name) {
		try {
			TupleSet table = catalog().primary().lookup(catalog().key().value(name));
			if (table.size() == 1) {
				return (Table) table.iterator().next().value(Catalog.SCHEMA.field("Table"));
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
	public Name name() {
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
	public abstract Index primary();
	
	/**
	 * @return All defined secondary indices.
	 */
	public abstract Hashtable<Key, Index> secondary();
	
	/**
	 * Add the remote foreign key constraint to this table.
	 * @param constraint The foreign key constraint
	 */
	private void remoteConstraint(ForeignKey constraint) {
		this.remoteConstraints.add(constraint);
	}
	
	protected void localConstraint(ForeignKey constraint) {
		this.localConstraints.add(constraint);
	}
	
	/**
	 *  Insert a tuple into this table. 
	 *  @return The delta set of tuples.
	 * @throws UpdateException 
	 **/
	public final void insert(TupleSet tuples, TupleSet delta, TupleSet conflict) throws UpdateException {
		for (Tuple t : tuples) {
			Tuple previous = insert(t);
			if (previous != null && previous.equals(t)) {
					continue;
			}
			else if (previous != null) {
				conflict.add(t);
			}
			delta.add(t);
		}
	}
	
	/**
	 *  Remove this tuple from the table. 
	 *  Note: This will only schedule the removal of committed tuples.
	 * @throws UpdateException 
	 **/
	public final void remove(TupleSet tuples, TupleSet delta) throws UpdateException {
		for (Tuple t : tuples) {
			if (remove(t)) {
				delta.add(t);
			}
		}
	}
	
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
