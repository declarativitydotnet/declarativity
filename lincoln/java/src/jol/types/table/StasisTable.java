package jol.types.table;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.util.Hashtable;
import java.util.Iterator;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;
import stasis.jni.LinearHash;

public class StasisTable extends Table {
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
    
    private static  byte[] toBytes(Object o) {
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
	public StasisTable(Runtime context, TableName name, Key key,
			TypeList attributeTypes) {
		super(name, Table.Type.TABLE, key, attributeTypes);

		byte[] nameBytes = toBytes(name);
		appendObject(key);
		appendObject(attributeTypes);
		byte[] schemaBytes = objectBytes();
		primary = new StasisIndex(runtime,this, this.key, Index.Type.PRIMARY);
		tbl = new LinearHash(nameBytes, schemaBytes);
	}

	private StasisTable(LinearHash catalog) {
		super(CATALOG_NAME, Type.TABLE, CATALOG_KEY, CATALOG_COLTYPES);
		primary = new StasisIndex(runtime,this, this.key, Index.Type.PRIMARY);
		
		tbl = new LinearHash(toBytes(CATALOG_NAME), CATALOG_SCHEMA_BYTES);
	}
	
	
	private static TableName CATALOG_NAME = new TableName("Stasis", "Catalog");
	private static Key CATALOG_KEY = new Key(0);
	private static TypeList CATALOG_COLTYPES = new TypeList(new Class[] { String.class, Long.class, Long.class} );
	private static byte[] CATALOG_SCHEMA_BYTES;
	private static Runtime runtime;
	
	private static StasisTable catalog = null;
	static { 
		appendObject(CATALOG_KEY);
		appendObject(CATALOG_COLTYPES);
		CATALOG_SCHEMA_BYTES = objectBytes();
	}
	
	LinearHash tbl;
	Index prim;
	Hashtable<Key,Index> sec;
	
	@Override
	public Integer cardinality() {
		return tbl.cardinality();
	}

	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		byte[] keybytes = toBytes(key.project(t));
		byte[] valbytes = toBytes(key.projectValue(t));
		return tbl.remove(keybytes, valbytes);
	}

	@Override
	protected boolean insert(Tuple t) throws UpdateException {
		try {
			byte[] keybytes= key.project(t).toBytes();
			byte[] valbytes = key.projectValue(t).toBytes();
			boolean ret = tbl.add(keybytes, valbytes);
			return ret;
		} catch(IOException e) {
			throw new UpdateException("Error serializing tuple", e);
		}
	}


	public Iterator<Tuple> tuples() {
		return new Iterator<Tuple>() {
			Iterator<byte[][]> it = tbl.tuples();

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

	@Override
	public Index primary() {
		return primary;
	}

	private StasisIndex primary;
	private Hashtable<Key, Index> secondary = new Hashtable<Key, Index>();
	
	@Override
	public Hashtable<Key, Index> secondary() {
		System.out.println("Stasis table: returning transient list of secondary indices:" + secondary.keySet().toString());
		return secondary;
	}

	public static void initializeStasis(Runtime runtime) {
		LinearHash.initialize(toBytes(CATALOG_NAME), CATALOG_SCHEMA_BYTES);
		StasisTable.runtime = runtime;
		catalog = new StasisTable(new LinearHash(toBytes(CATALOG_NAME), CATALOG_SCHEMA_BYTES)); 
	}
	public static StasisTable getCatalog(Runtime runtime) {
		if(catalog == null) initializeStasis(runtime);
		return catalog;
	}

}
