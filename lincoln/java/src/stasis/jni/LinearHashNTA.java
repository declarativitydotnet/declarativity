package stasis.jni;

import java.util.Arrays;
import java.util.Iterator;

import jol.core.Runtime;
import jol.lang.plan.Variable;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.basic.TypeList;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;
import jol.types.table.Key;
import jol.types.table.StasisTable;
import jol.types.table.TableName;

public class LinearHashNTA extends StasisTable {
	static LinearHashNTA catalog = null;

    protected Tuple header;
    private long[] rootRid;
    private long[] rid;
    
    protected Tuple registerTable(TableName name, Key key, TypeList type) throws UpdateException {
		Tuple header = new Tuple();
		header.append(new Variable("Page", Long.class), rid[0]);
		header.append(new Variable("Slot", Long.class), rid[1]);
		header.append(new Variable("Key", Key.class), key);
		header.append(new Variable("Types", TypeList.class), attributeTypes);
			
		Tuple nameTup = new Tuple();
		nameTup.append(new Variable("Name", TableName.class),name);
		
		Tuple row = catalog.key().reconstruct(nameTup, header);
		catalog.insert(row);
		
		return row;
		
    }
    protected LinearHashNTA(Runtime context) throws UpdateException {
		super(context, CATALOG_NAME, CATALOG_KEY, CATALOG_COLTYPES);
    	key = new Key(0);
		rootRid = Stasis.root_record();
		long type = Stasis.record_type(xid, rootRid);
		if(type == -1) {
			dirty = true;
			rid = Stasis.hash_create(xid);
			if(rid[0] != rootRid[0] || rid[1] != rootRid[1]) {
				throw new IllegalStateException();
			}
		} else {
			rid = new long[3];
			rid[0] = rootRid[0];
			rid[1] = rootRid[1];
		}
		header = CATALOG_SCHEMA;
	}
    
	public LinearHashNTA(Runtime context, TableName name, Key key,
			TypeList attributeTypes) throws UpdateException {
		super(context, name, key, attributeTypes);

		if(catalog == null) {
			catalog = new LinearHashNTA(context);  // open catalog based on recordid
			catalog.registerTable(CATALOG_NAME, CATALOG_KEY, CATALOG_COLTYPES);
		}
		Tuple nametup = new Tuple();
		nametup.append(new Variable("name", TableName.class),name);
		TupleSet headerSet;
		try {
			headerSet = catalog.primary().lookupByKey(nametup);
		} catch (BadKeyException e) {
			throw new IllegalStateException(e);
		}
		
		Tuple catalogEntry;
		if(headerSet.isEmpty()) {
			dirty = true;
			rid = Stasis.hash_create(xid);
			catalogEntry = registerTable(name, key, attributeTypes);
		} else {
			catalogEntry = headerSet.iterator().next();
			rid = new long[3];
		}
		header = catalog.primary().key().projectValue(catalogEntry);
		rid[0] = (Long)header.value(0);
		rid[1] = (Long)header.value(1);
	}

	@Override
	protected boolean add(byte[] keybytes, byte[] valbytes)
			throws UpdateException {
		dirty = true;
		byte[] oldvalbytes = Stasis.hash_insert(xid, rid, keybytes, valbytes);
		if(oldvalbytes != null && ! Arrays.equals(valbytes, oldvalbytes)) {
			throw new UpdateException("primary key violation");
		}
		return oldvalbytes == null;
	}
	
	@Override
	public Long cardinality() {
		return Stasis.hash_cardinality(-1, rid);
	}

	@Override
	protected boolean remove(byte[] keybytes, byte[] valbytes)
			throws UpdateException {
		dirty = true;
		byte[] oldvalbytes = Stasis.hash_remove(-1, rid, keybytes);
		if(oldvalbytes != null && ! Arrays.equals(valbytes, oldvalbytes)) {
			throw new UpdateException("primary key violation");
		}
		return oldvalbytes != null;
	}

	@Override
	protected byte[] lookup(byte[] keybytes) {
		return Stasis.hash_lookup(xid, rid, keybytes);
	}

	@Override
	protected Iterator<byte[][]> tupleBytes() {
		return new Iterator<byte[][]>() {
			private byte[] it = Stasis.hash_iterator(xid, rid);

			private byte[][] current = new byte[2][];
			private byte[][] next = new byte[2][];
			
			private boolean hadNext = true;
			Iterator<byte[][]> init() {
				hadNext = Stasis.iterator_next(xid, it);
				if(hadNext) {
					next[0] = Stasis.iterator_key(xid,it);
					next[1] = Stasis.iterator_value(xid, it);
					Stasis.iterator_tuple_done(xid, it);
					hadNext = Stasis.iterator_next(xid,it);
				} else {
					Stasis.iterator_close(xid, it);
				}
				return this;
			}
			
			public boolean hasNext() {
				return hadNext;
			}

			public byte[][] next() {
				if(hadNext) {
					current = next;
					next = new byte[2][];

					hadNext = Stasis.iterator_next(xid,it);
					if(hadNext) {
						next[0] = Stasis.iterator_key(xid,it);
						next[1] = Stasis.iterator_value(xid,it);
						Stasis.iterator_tuple_done(xid,it);
					} else {
						Stasis.iterator_close(xid, it);
					}
					return current;
				} else {
					throw new IllegalStateException("next() called after end of iterator");
				}
			}

			public void remove() {
				dirty = true;
				throw new UnsupportedOperationException("No support for removal via table iterators yet...");
			}

			@Override
			protected void finalize() throws Throwable {
				try {
					if(hadNext) 
						throw new IllegalStateException("detected non-exhausted iterator in finalize()");
				} finally {
					super.finalize();
				}
			}
		}.init();
	}

}
