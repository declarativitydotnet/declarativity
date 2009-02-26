package jol.types.table;

import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;
import jol.types.exception.UpdateException;
import stasis.jni.JavaHashtable;
import stasis.jni.Stasis;

public abstract class StasisTable extends Table {

	protected byte[] nameBytes;
	protected byte[] schemaBytes;
	protected StasisSerializer s = new StasisSerializer();
	protected TransactionStatus ts;
	public StasisTable(Runtime context, TableName name, Key key, Class[] attributeTypes) {
		super(name, Table.Type.TABLE, key, attributeTypes);
		synchronized (xactTable) {
			ts = xactTable.get(context.getPort());
	
			nameBytes = s.toBytes(name);
			s.appendObject(key);
			s.appendObject(attributeTypes);
			schemaBytes = s.objectBytes();
		}
		primary = new Index(context,this, this.key, Index.Type.PRIMARY, false) {
			@Override
			protected void insert(Tuple t) {
				throw new UnsupportedOperationException("Can't directly maniuplate clustered indices");
			}

			@Override
			public Iterator<Tuple> iterator() {
				return table().tuples().iterator(); 
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
	protected static Class[] CATALOG_COLTYPES = new Class[] { String.class, Long.class, Long.class};
	protected static Tuple CATALOG_SCHEMA = new Tuple(new Long(1), new Long(1), CATALOG_KEY, CATALOG_COLTYPES);
	protected static byte[] CATALOG_SCHEMA_BYTES;
	protected static byte[] CATALOG_NAME_BYTES; 
	//private static Runtime runtime;
	
	static { 
		CATALOG_SCHEMA_BYTES = new StasisSerializer().toBytes(CATALOG_SCHEMA);
		CATALOG_NAME_BYTES = new StasisSerializer().toBytes(CATALOG_NAME);
	}
	
	Index prim;
	
	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		try {
			byte[] keybytes = key.project(t).toBytes();
			byte[] valbytes = key.projectValue(t).toBytes();
			return remove(keybytes, valbytes);
		} catch(UpdateException e) {
			throw new UpdateException("Error deleting tuple " + t + " from " + this, e);
		} catch(IOException e) {
			throw new UpdateException("Error serializing tuple" + t, e);
		}
	}

	@Override
	protected boolean insert(Tuple t) throws UpdateException {
		try {
			byte[] keybytes= key.project(t).toBytes();
			byte[] valbytes = key.projectValue(t).toBytes();
			return add(keybytes, valbytes);
		} catch(UpdateException e) {
			throw new UpdateException("Error inserting tuple " + t + " into " + this, e);
		} catch(IOException e) {
			throw new UpdateException("Error serializing tuple", e);
		}
	}


	@Override
	public Iterable<Tuple> tuples() {
		return new Iterable<Tuple>() {
			public Iterator<Tuple> iterator() {
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
							
							return key.reconstruct(k, v);
						} catch (Exception e) {
							throw new Error("couldn't deserialize", e);
						}
					}

					public void remove() {
						it.remove();
					}
				};
			}			
		};
	}

	final StasisTable thisTable = this;	
	private Index primary;
	final private Map<Key, Index> secondary = new HashMap<Key, Index>() {
        private static final long serialVersionUID = 1L;

		@Override
        public Index put(Key k, Index i) {
			int c = 0;
			System.err.println("rebuilding index of " + name() +"...");
			
			for(Tuple t: thisTable.tuples()) {
				System.err.println("\t" + t);
				i.insert(t); c++;
			}
			System.err.println("   ...done ("+c+" tuples found)");
			return super.put(k,i);
		}
	};

	@Override
	public Index primary() {
		return primary;
	}

	@Override
	public Map<Key, Index> secondary() {
		return secondary;
	}

	protected static class TransactionStatus {
		public long xid;
		public boolean dirty;
		public TransactionStatus(long xid) { this.xid = xid; this.dirty = false; }
	};
	
	protected static final Map<Integer,TransactionStatus> xactTable 
		= new HashMap<Integer,TransactionStatus>();
	
	public static boolean foundStasis = false;
	
	public static void initializeStasis(Runtime context) {
		JavaHashtable.initialize(new StasisSerializer().toBytes(CATALOG_NAME), CATALOG_SCHEMA_BYTES);
		try {
			Stasis.loadLibrary();
			foundStasis = true;
		} catch (UnsatisfiedLinkError e) {
		}
		synchronized (xactTable) {
			if(foundStasis) {
				System.err.print("Stasis recovery..."); System.err.flush();
				int initcount = Stasis.init();
				if(initcount == 1) {
					System.err.println("suceeded");
				} else {
					System.err.println("skipped; attached to running copy of Stasis");
				}
				TransactionStatus newTs = new TransactionStatus(Stasis.begin());
				xactTable.put(context.getPort(), newTs);
			}
		}
	}
	public static void deinitializeStasis(Runtime context) {
		synchronized(xactTable) {
			if(foundStasis) {
				TransactionStatus s = xactTable.get(context.getPort());
				if(s.dirty) {
					Stasis.commit(s.xid);
					xactTable.remove(context.getPort());
				}
				if(xactTable.size() == 0) {
					Stasis.deinit();
				}
			}
		}
	}
	
	public static void commit(Runtime context) {
		synchronized(xactTable) {
			TransactionStatus s = xactTable.get(context.getPort());
			if(foundStasis && s.dirty) {
				System.err.println("commit transaction " + s.xid);
				Stasis.commit(s.xid);
				s.xid = Stasis.begin();
				s.dirty = false;
			}
		}
	}
	// XXX figure out how to abort transactions
	public static void abort(Runtime context) {
		synchronized (xactTable) {
			TransactionStatus s = xactTable.get(context.getPort());
			if(foundStasis && s.dirty) {
				Stasis.abort(s.xid);
				s.xid = Stasis.begin();
				s.dirty = false;
			}
		}
	}

	@Override
	public abstract Long cardinality();
    protected abstract boolean remove(byte[] keybytes, byte[] valbytes) throws UpdateException;
	protected abstract boolean add(byte[] keybytes, byte[] valbytes) throws UpdateException;
	protected abstract byte[] lookup(byte[] keybytes);
	protected abstract Iterator<byte[][]> tupleBytes();
}
