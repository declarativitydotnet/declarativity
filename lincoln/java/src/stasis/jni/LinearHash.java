package stasis.jni;

import java.util.Arrays;
import java.util.Hashtable;
import java.util.Iterator;

import jol.types.exception.UpdateException;

public class LinearHash {
	private static Hashtable<byte[], byte[]> catalog;
	private static Hashtable<byte[], Hashtable<byte[], byte[]>> tables;

	public static void initialize(byte [] catalogName, byte[] catalogSchema) {
		assert(catalog == null);
	
		catalog = new Hashtable<byte[],byte[]>();
		tables = new Hashtable<byte[], Hashtable<byte[], byte[]>>();
		tables.put(catalogName, catalog);
		catalog.put(catalogName, catalogSchema);
	}
	
	private final Hashtable<byte[], byte[]> tbl;
	
	public LinearHash(byte[] name, byte[] schema) {
		if(catalog.contains(name)) {
			assert(Arrays.equals(schema, catalog.get(name)));
		} else {
			catalog.put(name,schema);
			tables.put(name,new Hashtable<byte[], byte[]>());
		}
		this.tbl = tables.get(name);
	}

	public boolean remove(byte[] keybytes, byte[] valbytes) throws UpdateException {
		byte[] oldvalbytes = tbl.remove(keybytes);
		if(oldvalbytes != null && ! Arrays.equals(valbytes, oldvalbytes)) {
			throw new UpdateException("primary key violation");
		}
		return oldvalbytes != null;
	}

	public boolean add(byte[] keybytes, byte[] valbytes) throws UpdateException {
		byte[] oldvalbytes = tbl.put(keybytes, valbytes);
		if(oldvalbytes != null && ! Arrays.equals(valbytes, oldvalbytes)) {
			throw new UpdateException("primary key violation");
		}
		return oldvalbytes == null;
	}

	public Integer cardinality() {
		return tbl.size();
	}

	public Iterator<byte[][]> tuples() {
		return new Iterator<byte[][]>() {
			Iterator<byte[]> it = tbl.keySet().iterator();

			public boolean hasNext() {
				return it.hasNext();
			}

			public byte[][] next() {
				byte[] ret = it.next();
				if(ret == null) { return null; }
				byte[][] retarray = new byte[2][];
				retarray[0] = ret;
				retarray[1] = tbl.get(ret);
				return retarray;
			}

			public void remove() {
				it.remove();
			}
		};
	}
}
