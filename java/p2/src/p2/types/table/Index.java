package p2.types.table;

import java.lang.reflect.Constructor;
import java.util.Iterator;
import java.util.Vector;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.BadKeyException;
import p2.types.exception.UpdateException;


public abstract class Index implements Comparable<Index>, Iterable<Tuple> {
	
	private class Listener extends Table.Callback {
		public void deletion(TupleSet tuples) {
			for (Tuple tuple : tuples) {
				remove(tuple);
			}
		}
		public void insertion(TupleSet tuples) {
			for (Tuple tuple : tuples) {
				insert(tuple);
			}
		}
	}
	
	public static class IndexTable extends ObjectTable {
		private static final Key PRIMARY_KEY = new Key(0,1); 
		
		public enum Field {TABLENAME, KEY, TYPE, CLASSNAME, OBJECT};
		private static final Class[] SCHEMA = { 
			String.class, // table name
			Key.class,    // key
			Type.class,   // the type
			String.class, // the class type
			Index.class   // the object
		};
		
		public IndexTable() {
			super("index", PRIMARY_KEY, new TypeList(SCHEMA));
		}
		
		@Override
		public boolean insert(Tuple tuple) throws UpdateException {
			if (tuple.value(Field.OBJECT.ordinal()) == null) {
				try {
					Class ctype = Class.forName((String)tuple.value(Field.CLASSNAME.ordinal()));
					Constructor constructor = ctype.getConstructor(Table.class, Key.class, Type.class);
					Table table = Table.table((String)tuple.value(Field.TABLENAME.ordinal()));
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
	
	public enum Type{PRIMARY, SECONDARY};
	
	private Table table;
	
	/* The index key referencing attribute positions in the indexed table. */
	private Key key;
	
	private Type type;
	
	public Index(Table table, Key key, Type type) {
		this.table = table;
		this.key = key;
		this.type = type;
		IndexTable iTable = Table.index();
		if (iTable != null) {
			try {
				iTable.insert(new Tuple(iTable.name(), table.name(), key, type, this.getClass().getName(), this));
			} catch (UpdateException e) {
				e.printStackTrace();
				System.exit(1);
			}
		}
		this.table.register(new Listener());
	}
	
	public abstract String toString();
	
	public int compareTo(Index i) {
		return table().name().compareTo(i.table().name());
	}
	
	public Table table() {
		return this.table;
	}
	
	public Type type() {
		return this.type;
	}
	
	public Key key() {
		return key;
	}
	
	/**
	 * Clear out all tuples from this index.
	 */
	public abstract void clear();
	
	/* An iterator over all tuple values. */
	public abstract Iterator<Tuple> iterator();
	
	/* Uses the index key as the lookup key and
	 * the values from the given tuple. */
	public abstract TupleSet lookup(Tuple t) throws BadKeyException;
	
	public abstract TupleSet lookup(Comparable... values) throws BadKeyException;
	
	/* Index the given tuple. */
	protected abstract void insert(Tuple t);
	
	/* Remove the tuple from the index. */
	protected abstract void remove(Tuple t);

}
