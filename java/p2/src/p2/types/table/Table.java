package p2.types.table;

import java.lang.reflect.Constructor;
import java.util.*;

import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.BadKeyException;
import p2.types.exception.UpdateException;
import p2.types.table.Key.Value;
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
			return this.name;
		}
		
		public int compareTo(Object o) {
			String name = (o instanceof Name) ? 
					      ((Name)o).toString() : 
					    	  ((o instanceof String) ? (String) o : "");
			return name.compareTo(toString());
		}

		public String object() {
			return this.object;
		}
	}
	
	public static class Catalog extends ObjectTable {
		private static final Key PRIMARY_KEY = new Key(0);

		private static final Schema SCHEMA = 
			new Schema(new Schema.Entry("Name",  Name.class),
					new Schema.Entry("Schema",   Schema.class),
					new Schema.Entry("Size",     Integer.class),
					new Schema.Entry("Lifetime", Integer.class),
					new Schema.Entry("Key",      Key.class),
					new Schema.Entry("Table",    Table.class));
		
		private SymbolTable symbolTable;

		private Catalog() {
			super(new Name("catalog", Catalog.class.getName()), SCHEMA, PRIMARY_KEY);
			this.symbolTable = new SymbolTable();
			try {
				super.insert(new Tuple("catalog", new Name("catalog", Catalog.class.getName()), 
						              SCHEMA, INFINITY, INFINITY, this));
			} catch (UpdateException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		public SymbolTable symbolTable() {
			return this.symbolTable;
		}

		@Override
		protected Tuple insert(Tuple tuple) throws UpdateException {
			Table table = null;
			Name   name   = (Name)   tuple.value(SCHEMA.field("Name"));
			Schema schema = (Schema) tuple.value(SCHEMA.field("Schema"));
			try {
				Class eclass = Class.forName(name.object);
				Constructor constructor = eclass.getConstructor(String.class, Schema.class, Integer.class, Number.class, Key.class);
				table = (Table) constructor.newInstance(name.object, 
						                                name,
						                                schema,
						                                tuple.value(SCHEMA.field("Size")),  
						                                tuple.value(SCHEMA.field("Lifetime")), 
						                                tuple.value(SCHEMA.field("Key")));
				symbolTable.root().define(name.toString(), schema.types());
			} catch (Exception e) {
				throw new UpdateException(e.toString());
			}
			assert (table != null);
			tuple.value(SCHEMA.field("Table"), table);
			
			return super.insert(tuple);
		}
		
	}

	private final static Catalog CATALOG = new Catalog();
	
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
	
	private Index primary;
	
	private Hashtable<Key, Index> secondary;
	
	protected Table(Name name, Schema schema, Integer size, Number lifetime, Key key) {
		this.name = name;
		this.schema = schema;
		this.size = size;
		this.lifetime = lifetime;
		this.key = key;
	}
	
	public String toString() {
		String value = "Table: " + name.toString() + ", " + schema.toString() + 
		        ", " + size + ", " + lifetime + ", " + key + "\n";
		for (Tuple t : this) {
			value += t.toString() + "\n";
		}
		return value;
	}
	
	public Iterator<Tuple> iterator() {
		return primary().tuples();
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
		TupleSet conflicts = new SimpleTupleSet(name().toString(), schema());
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
		TupleSet delta = new SimpleTupleSet(name().toString(), schema());
		for (Tuple t : tuples) {
			Tuple previous = insert(t);
			if (previous != null && !previous.equals(t)) {
				delta.add(t);
			}
		}
		return delta;
	}
	
	/**
	 * Remove this tuple from the table. 
	 * Note: This will only schedule the removal of committed tuples.
	 * @throws UpdateException 
	 **/
	public final TupleSet remove(TupleSet tuples) throws UpdateException {
		TupleSet delta = new SimpleTupleSet(name().toString(), schema());
		for (Tuple t : tuples) {
			if (remove(t)) {
				delta.add(t);
			}
		}
		return delta;
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
