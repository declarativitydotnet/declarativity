package jol.types.table;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.util.Hashtable;
import java.util.Iterator;

import jol.core.Runtime;
import jol.lang.plan.Variable;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;
import stasis.jni.JavaHashtable;
import stasis.jni.Stasis;

public abstract class StasisTable extends Table {
	private static final ByteArrayOutputStream outArray = new ByteArrayOutputStream();
    private static ObjectOutputStream serializer;

    static {
    	ObjectOutputStream s;
    	try {
    		s = new ObjectOutputStream(outArray);
    	} catch(IOException e) {
    		e.printStackTrace();
    		s = null;
    	}
    	serializer = s;
    }
    
    protected static  byte[] toBytes(Object o) {
		appendObject(o);
		return objectBytes();
	}

	private static  void appendObject(Object o) {
		try {
			serializer.writeObject(o);
		} catch(IOException e) {
			e.printStackTrace();
			System.exit(-1);
		}
	}
	private static byte[] objectBytes() {
		try {
			serializer.close(); // does not affect outArray...
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(-1);
		}
		byte[] ret = outArray.toByteArray(); // makes copy
		outArray.reset();  // reset outArray to beginning of stream for next time.
		try {
			serializer = new ObjectOutputStream(outArray);
		} catch(IOException e) {
			e.printStackTrace();
			System.exit(-1);
		}
		if(ret == null) {
			System.out.println("Impossible!!!");
			System.exit(-1);
		}
		return ret;
	}

	protected byte[] nameBytes;
	protected byte[] schemaBytes;
	
	public StasisTable(Runtime context, TableName name, Key key,
			TypeList attributeTypes) {
		super(name, Table.Type.TABLE, key, attributeTypes);

		nameBytes = toBytes(name);
		appendObject(key);
		appendObject(attributeTypes);
		schemaBytes = objectBytes();
		
		primary = new Index(context,this, this.key, Index.Type.PRIMARY, false) {
			@Override
			protected void insert(Tuple t) {
				throw new UnsupportedOperationException("Can't directly maniuplate clustered indices");
			}

			@Override
			public Iterator<Tuple> iterator() {
				return table().tuples(); 
			}

			@Override
			public TupleSet lookupByKey(Tuple key) throws BadKeyException {
				try {
					byte[] valbytes = ((StasisTable)table()).lookup(key.toBytes());
					TupleSet ret = new TupleSet();
					if(valbytes != null) {
						Tuple t = key().reconstruct(key, new Tuple(valbytes));
						ret.add(t);
					}
					return ret;
				} catch (IOException e) {
					throw new IllegalStateException("can't happen");
				}
			}

			@Override
			protected void remove(Tuple t) {
				throw new UnsupportedOperationException("Can't directly maniuplate clustered indices");
			}

			@Override
			public String toString() {
				return "instance of " + getClass().toString();
			}
			
		};
	}

	protected static TableName CATALOG_NAME = new TableName("Stasis", "Catalog");
	protected static Key CATALOG_KEY = new Key(0);
	protected static TypeList CATALOG_COLTYPES = new TypeList(new Class[] { String.class, Long.class, Long.class} );
	protected static Tuple CATALOG_SCHEMA = new Tuple();
	protected static byte[] CATALOG_SCHEMA_BYTES;
	protected static byte[] CATALOG_NAME_BYTES; 
	private static Runtime runtime;
	
	static { 
		CATALOG_SCHEMA.append(new Variable("Page", Long.class), new Long(1));
		CATALOG_SCHEMA.append(new Variable("Slot", Long.class), new Long(1));
		CATALOG_SCHEMA.append(new Variable("Key", Key.class), CATALOG_KEY);
		CATALOG_SCHEMA.append(new Variable("Types", TypeList.class), CATALOG_COLTYPES);
		CATALOG_SCHEMA_BYTES = toBytes(CATALOG_SCHEMA);
		CATALOG_NAME_BYTES = toBytes(CATALOG_NAME);
	}
	
	//LinearHash tbl;
	Index prim;
	Hashtable<Key,Index> sec;
	
	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		byte[] keybytes = toBytes(key.project(t));
		byte[] valbytes = toBytes(key.projectValue(t));
		return remove(keybytes, valbytes);
	}

	@Override
	protected boolean insert(Tuple t) throws UpdateException {
		try {
			byte[] keybytes= key.project(t).toBytes();
			byte[] valbytes = key.projectValue(t).toBytes();
			boolean ret = add(keybytes, valbytes);
			return ret;
		} catch(IOException e) {
			throw new UpdateException("Error serializing tuple", e);
		}
	}


	@Override
	public Iterator<Tuple> tuples() {
		return new Iterator<Tuple>() {
			Iterator<byte[][]> it = tupleBytes();

			public boolean hasNext() {
				return it.hasNext();
			}

			public Tuple next() {
				byte[][] o = it.next();
				try {
					Tuple k = new Tuple(o[0]);
					Tuple v = new Tuple(o[1]);
					
					Tuple ret =key.reconstruct(k, v);
								
					return ret;
				} catch (Exception e) {
					throw new Error("couldn't deserialize", e);
				}
			}

			public void remove() {
				it.remove();
			}
		};
	}

	
	private Index primary;
	private Hashtable<Key, Index> secondary = new Hashtable<Key, Index>();

	@Override
	public Index primary() {
		return primary;
	}

	@Override
	public Hashtable<Key, Index> secondary() {
		System.out.println("Stasis table: returning transient list of secondary indices:" + secondary.keySet().toString());
		return secondary;
	}

	// XXX add support for concurrent transactions
	protected static long xid;
	protected static boolean dirty = false;
	public static boolean foundStasis = false;
	
	public static void initializeStasis(Runtime runtime) {
		JavaHashtable.initialize(toBytes(CATALOG_NAME), CATALOG_SCHEMA_BYTES);
		StasisTable.runtime = runtime;
		try {
			Stasis.loadLibrary();
			foundStasis = true;
		} catch (UnsatisfiedLinkError e) {
		}
		if(foundStasis) {
			System.err.print("Stasis recovery..."); System.err.flush();
			Stasis.init();
			System.err.println("suceeded");
			xid = Stasis.begin();
			foundStasis = true;
		}
	}
	public static void deinitializeStasis() {
		if(foundStasis) {
			if(dirty) {
				Stasis.commit(xid);
			}
			Stasis.deinit();
		}
	}
	
	public static void commit() {
		if(foundStasis && dirty) {
			System.err.println("commit transaction " + xid);
			Stasis.commit(xid);
			xid = Stasis.begin();
			dirty = false;
		}
	}
	// XXX figure out how to abort transactions
	public static void abort() {
		if(foundStasis && dirty) {
			Stasis.abort(xid);
			xid = Stasis.begin();
			dirty = false;
		}
	}

	@Override
	public abstract Long cardinality();
    protected abstract boolean remove(byte[] keybytes, byte[] valbytes) throws UpdateException;
	protected abstract boolean add(byte[] keybytes, byte[] valbytes) throws UpdateException;
	protected abstract byte[] lookup(byte[] keybytes);
	protected abstract Iterator<byte[][]> tupleBytes();
}
