package types.table;

import java.util.Iterator;
import java.util.Vector;
import types.basic.Tuple;
import types.basic.TupleSet;
import types.exception.UpdateException;


public abstract class Index {
	
	public static class IndexTable extends ObjectTable {
		private static final Key PRIMARY_KEY = new Key(0,1); 
		
		private static final Schema SCHEMA = 
			new Schema(new Schema.Entry("Name",     String.class),
					   new Schema.Entry("Key",      Integer[].class),
					   new Schema.Entry("Type",     Index.Type.class),
					   new Schema.Entry("Method",   Index.Method.class));
		
		public IndexTable(Table.Name name, Schema schema, Integer size, Number lifetime, Key key) {
			super(name, schema, key);
		}
		
		@Override
		public Tuple insert(Tuple tuple) throws UpdateException {
			// TODO Auto-generated method stub
			return super.insert(tuple);
		}

		@Override
		public boolean remove(Tuple tuple) throws UpdateException {
			// TODO Auto-generated method stub
			return super.remove(tuple);
		}

	}
	
	public enum Type {PRIMARY, SECONDARY};
	
	public enum Method {HASH};
	
	private Type type;
	
	private Method method;
	
	/* The index key referencing attribute positions in the indexed table. */
	private Key key;
	
	private static IndexTable INDEX = null;
	
	public Index(Type type, Method method, Key key) {
		this.type = type;
		this.method = method;
		this.key = key;
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
	
	public Type type() {
		return this.type;
	}
	
	public Method method() {
		return this.method;
	}
	
	public Key key() {
		return key;
	}
	
	/* The table over which this index is defined. */
	public abstract Table table();
	
	/* Uses the index key as the lookup key and
	 * the values from the given tuple. */
	public abstract TupleSet lookup(Tuple t);
	
	public abstract TupleSet lookup(Key.Value key);
	
	public abstract boolean containsKey(Key.Value key);
	
	/* Index the given tuples. */
	public abstract Tuple insert(Tuple t);
	
	/* Remove this tuple from the index. */
	public abstract void remove(Tuple t);
	
	/* Validate this index. */
	public abstract boolean commit();
	
	/* An iterator over all keys in the index. */
	public abstract Iterator<Key.Value> keys();
	
	/* An iterator over all tuple values. */
	public abstract Iterator<Tuple> tuples();
	
}
