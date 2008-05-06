package p2.types.table;

import java.util.*;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.UpdateException;
import xtc.util.SymbolTable;

public abstract class Table implements Iterable<Tuple>, Comparable {
	public static final Integer INFINITY = Integer.MAX_VALUE;

	public static class Catalog extends ObjectTable {
		private static final Key PRIMARY_KEY = new Key(0);

		public enum Field {TABLENAME, SIZE, LIFETIME, KEY, TYPES, OBJECT};
		private static final Class[] SCHEMA = { 
			String.class,    // The tablename
			Integer.class,   // The table size
			Float.class,    // The lifetime
			Key.class,       // The primary key
			TypeList.class,  // The type of each attribute
			Table.class      // The table object
		};
		
		private SymbolTable symbolTable;

		public Catalog() {
			super("catalog", PRIMARY_KEY, new TypeList(SCHEMA));
			this.symbolTable = new SymbolTable();
		}
		
		public SymbolTable symbolTable() {
			return this.symbolTable;
		}

		@Override
		protected Tuple insert(Tuple tuple) throws UpdateException {
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

	/* The table name. */
	private String name;
	
	/* The number of attributes in this table. */
	private TypeList attributeTypes;
	
	/* The maximum size of this table. */
	private Integer size;
	
	/* the lifetime of a tuple, in seconds. */
	private Float lifetime;
	
	/* The primary key. */
	private Key key;
	
	private Index primary;
	
	private Hashtable<Key, Index> secondary;
	
	protected Table(String name, Integer size, Float lifetime, Key key, TypeList attributeTypes) {
		this.name = name;
		this.attributeTypes = attributeTypes;
		this.size = size;
		this.lifetime = lifetime;
		this.key = key;
		
		if (p2.core.System.catalog() == null) {
			/* I am the catalog! */
			try {
				insert(new Tuple(name, name, size, lifetime, key, attributeTypes, this));
			} catch (UpdateException e) {
				e.printStackTrace();
				System.exit(0);
			}
		}
		else {
			Table.register(name, size, lifetime, key, attributeTypes, this);
		}
	}
	
	public String toString() {
		String value = "Table: " + name.toString() + ", " + attributeTypes.toString() + 
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
	
	private static void register(String name, Integer size, Float lifetime, 
			                     Key key, TypeList types, Table object) { 
		
		Tuple tuple = new Tuple(p2.core.System.catalog().name(), name, size, lifetime, key, types, object);
		try {
			p2.core.System.catalog().insert(tuple);
		} catch (UpdateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(0);
		}
	}
	
	public static boolean drop(String name) throws UpdateException {
		if (p2.core.System.catalog() == null) {
			throw new UpdateException("Catalog has not been created!");
		}
		
		try {
			TupleSet tuples = p2.core.System.catalog().primary().lookup(p2.core.System.catalog().key().value(name));
			for (Tuple table : tuples) {
				return p2.core.System.catalog().remove(table);
			}
		} catch (BadKeyException e) {
			// TODO Fatal error
			e.printStackTrace();
		}
		return false;
	}
	
	public static Table table(String name) {
		try {
			TupleSet table = p2.core.System.catalog().primary().lookup(p2.core.System.catalog().key().value(name));
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
		return (Class[]) attributeTypes.toArray();
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
		TupleSet delta = new TupleSet(name().toString());
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
