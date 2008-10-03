package p2.types.table;

import java.util.Hashtable;
import java.util.Iterator;
import java.util.Vector;

import p2.types.basic.Schema;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.basic.TypeList;
import p2.types.exception.UpdateException;


public class BasicTable extends Table {
	/* The primary key. */
	protected Key key;
	
	protected TupleSet tuples;
	
	protected Index primary;
	
	protected Hashtable<Key, Index> secondary;
	
	public BasicTable(TableName name, Key key, TypeList types) {
		super(name, Type.TABLE, key, types);
		this.key = key;
		this.tuples = new TupleSet(name);
		this.primary = new HashIndex(this, key, Index.Type.PRIMARY);
		this.secondary = new Hashtable<Key, Index>();
	}
	
	@Override
	public TupleSet tuples() {
		try {
		return this.tuples == null ? new TupleSet(name()) : this.tuples.clone();
		} catch (Exception e) {
			System.err.println("TABLE " + name() + " ERROR: " + e);
			e.printStackTrace();
			System.exit(0);
		}
		return null;
	}
	
	@Override
	protected boolean insert(Tuple t) throws UpdateException {
		return this.tuples.add(t);
	}
	
	@Override
	protected boolean delete(Tuple t) throws UpdateException {
		return this.tuples.remove(t);
	}

	@Override
	public Index primary() {
		return this.primary;
	}

	@Override
	public Hashtable<Key, Index> secondary() {
		return this.secondary;
	}

	@Override
	public Integer cardinality() {
		return this.tuples().size();
	}

}
