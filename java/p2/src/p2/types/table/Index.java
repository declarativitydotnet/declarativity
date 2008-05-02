package p2.types.table;

import java.lang.reflect.Constructor;
import java.util.Iterator;
import java.util.Vector;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.exception.UpdateException;


public abstract class Index implements Comparable {
	
	public static class IndexTable extends ObjectTable {
		private static final Key PRIMARY_KEY = new Key(0,1); 
		
		private static final Schema SCHEMA = 
			new Schema(new Schema.Entry("Table",   String.class),
					   new Schema.Entry("Key",     Key.class),
					   new Schema.Entry("Type",    Type.class),
					   new Schema.Entry("Class",   String.class),
					   new Schema.Entry("Object",  Index.class));
		
		public IndexTable(Table.Name name, Schema schema, Integer size, Number lifetime, Key key) {
			super(name, schema, key);
		}
		
		@Override
		public Tuple insert(Tuple tuple) throws UpdateException {
			if (tuple.value(SCHEMA.field("Object")) == null) {
				try {
					Class ctype = Class.forName((String)tuple.value(SCHEMA.field("Class")));
					Constructor constructor = ctype.getConstructor(Table.class, Key.class, Type.class);
					Table table = Table.table((String)tuple.value(SCHEMA.field("Table")));
					Key key = (Key) tuple.value(SCHEMA.field("Key"));
					Type type = (Type)tuple.value(SCHEMA.field("Type"));
					Index index = (Index) constructor.newInstance(table, key, type);
					tuple.value(SCHEMA.field("Object"), index);
				} catch (Exception e) {
					throw new UpdateException(e.toString());
				}
			}
			return super.insert(tuple);
		}

		@Override
		public boolean remove(Tuple tuple) throws UpdateException {
			// TODO Auto-generated method stub
			return super.remove(tuple);
		}

	}
	
	public enum Type{PRIMARY, SECONDARY};
	
	private Table table;
	
	/* The index key referencing attribute positions in the indexed table. */
	private Key key;
	
	private Type type;
	
	private static IndexTable INDEX = null;
	
	public Index(Table table, Key key, Type type) {
		this.table = table;
		this.key = key;
		this.type = type;
	}
	
	public static void initialize() {
		if (INDEX == null) {
			try {
				INDEX = (IndexTable) 
				          Table.create(new Table.Name("index", IndexTable.class.getName()), 
				        		       IndexTable.SCHEMA, IndexTable.PRIMARY_KEY);
			} catch (UpdateException e) {
				e.printStackTrace();
				java.lang.System.exit(0);
			}
		}
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
	
	/* An iterator over all tuple values. */
	public abstract Iterator<Tuple> tuples();
	
	/* Uses the index key as the lookup key and
	 * the values from the given tuple. */
	public abstract TupleSet lookup(Tuple t);
	
	public abstract TupleSet lookup(Key.Value key);
	
	/* Index the given tuple. */
	public abstract Tuple insert(Tuple t);
	
	/* Remove the tuple from the index. */
	public abstract void remove(Tuple t);
	
}
