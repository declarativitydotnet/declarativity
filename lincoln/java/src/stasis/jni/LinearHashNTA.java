package stasis.jni;

import java.io.IOException;
import java.util.Arrays;
import java.util.Iterator;

import jol.core.Runtime;
import jol.lang.plan.Variable;
import jol.types.basic.Tuple;
import jol.types.basic.TypeList;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.StasisTable;
import jol.types.table.TableName;

public class LinearHashNTA extends StasisTable {
	static LinearHashNTA catalog = null;

    protected Tuple header;
    private long page;
    private long slot;
    
	protected  void registerTable(long xid, byte[] name, byte[] schema, long page, long slot)
		throws UpdateException {
		catalog.add(name,schema);
	}
    protected byte[] registerTable(long xid, TableName name, long page, long slot, Key key, TypeList type) throws UpdateException {
		Tuple header = new Tuple();
		header.append(new Variable("Page", Long.class), page);
		header.append(new Variable("Slot", Long.class), slot);
		header.append(new Variable("Key", Key.class), key);
		header.append(new Variable("Types", TypeList.class), attributeTypes);
		try {
			byte[] headerBytes = header.toBytes();
			registerTable(xid, toBytes(name), headerBytes, 1, 1);
			return headerBytes;
		} catch(IOException e) { 
			throw new IllegalStateException(e);
		}
    }
	
    protected LinearHashNTA(Runtime context, long xid) throws UpdateException {
		super(context, CATALOG_NAME, CATALOG_KEY, CATALOG_COLTYPES);
		long type = Stasis.record_type_read(xid, 1, 1);
		if(type == -1) {
			long[] ret = Stasis.hash_create(xid);
			if(ret[0] != 1 || ret[1] != 1) {
				throw new IllegalStateException();
			}
			registerTable(xid, CATALOG_NAME_BYTES, CATALOG_SCHEMA_BYTES, 1, 1);
		}
		header = CATALOG_SCHEMA;
	}
    
	public LinearHashNTA(Runtime context, TableName name, Key key,
			TypeList attributeTypes) throws UpdateException {
		super(context, name, key, attributeTypes);

		if(catalog == null) {
			catalog = new LinearHashNTA(context, -1);  // open catalog based on recordid
		}
		
		byte[] tableHeader = 
			Stasis.hash_lookup(-1, 1, 1, toBytes(name));

		if(tableHeader == null) {
			long [] rid = Stasis.hash_create(-1);
			tableHeader = registerTable(-1, name, rid[0], rid[1], key, attributeTypes);
		} 
		try {
			header = new Tuple(tableHeader);
			page = (Long)header.value(0);
			slot = (Long)header.value(1);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	protected boolean add(byte[] keybytes, byte[] valbytes)
			throws UpdateException {
		byte[] oldvalbytes = Stasis.hash_insert(-1, page, slot, keybytes, valbytes);
		if(oldvalbytes != null && ! Arrays.equals(valbytes, oldvalbytes)) {
			throw new UpdateException("primary key violation");
		}
		return oldvalbytes != null;
	}

	@Override
	public Long cardinality() {
		return Stasis.hash_cardinality(-1, page, slot);
	}

	@Override
	protected boolean remove(byte[] keybytes, byte[] valbytes)
			throws UpdateException {
		byte[] oldvalbytes = Stasis.hash_remove(-1, page, slot, keybytes);
		if(oldvalbytes != null && ! Arrays.equals(valbytes, oldvalbytes)) {
			throw new UpdateException("primary key violation");
		}
		return oldvalbytes != null;
	}

	@Override
	protected byte[] lookup(byte[] keybytes) {
		return Stasis.hash_lookup(-1L, page, slot, keybytes);
	}

	@Override
	protected Iterator<byte[][]> tupleBytes() {
		return new Iterator<byte[][]>() {

			private byte[] it = Stasis.hash_iterator(-1, page, slot);

			private byte[][] current = new byte[2][];
			private byte[][] next = new byte[2][];
			
			Iterator<byte[][]> init() {
				boolean hadNext = Stasis.iterator_next(it);
				if(hadNext) {
					current[0] = Stasis.iterator_key(it);
					current[1] = Stasis.iterator_value(it);
					Stasis.iterator_tuple_done(it);
					
					hadNext = Stasis.iterator_next(it);
					if(hadNext) {
						next[0] = Stasis.iterator_key(it);
						next[1] = Stasis.iterator_value(it);
						Stasis.iterator_tuple_done(it);
					}
				}
				return this;
			}
			
			@Override
			public boolean hasNext() {
				return next[0] != null;
			}

			@Override
			public byte[][] next() {
				if(next[0] == null) {
					throw new IllegalStateException("next() called after end of iterator");
				} else {
					current = next;
					byte[][] next = new byte[2][];
					boolean hadNext = Stasis.iterator_next(it);
					if(hadNext) {
						next[0] = Stasis.iterator_key(it);
						next[1] = Stasis.iterator_value(it);
						Stasis.iterator_tuple_done(it);
					}
					return current;
				}
			}

			@Override
			public void remove() {
				throw new UnsupportedOperationException("No support for removal via table iterators yet...");
			}
			protected void finalize() throws Throwable {
				try {
					Stasis.iterator_close(it);
				} finally {
					super.finalize();
				}
			}
		}.init();
	}

}
