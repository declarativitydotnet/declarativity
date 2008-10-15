package jol.types.table;

import java.util.HashMap;
import java.util.Iterator;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;

public class StasisIndex extends Index {

	HashMap<Tuple, Tuple> idx;
	Key key;
	
	public StasisIndex(Runtime context, Table table, Key key, jol.types.table.Index.Type type) {
		super(context, table, key, type);
		idx = new HashMap<Tuple,Tuple>();
		this.key = key;
	}

	@Override
	public void clear() {
		idx.clear();
	}

	@Override
	protected void insert(Tuple t) {
		idx.put(key.project(t), key.projectValue(t));
	}

	@Override
	public Iterator<Tuple> iterator() {
		return new Iterator<Tuple>() {
			Iterator<Tuple> ks = idx.keySet().iterator();
			@Override
			public boolean hasNext() {
				return ks.hasNext();
			}

			@Override
			public Tuple next() {
				Tuple next = ks.next();
				if(next == null) return null;
				return key.reconstruct(next, idx.get(next));
			}

			@Override
			public void remove() {
				ks.remove();
			}
		
		};
	}

	@Override
	public TupleSet lookup(Tuple t) throws BadKeyException {
		Tuple ret = idx.get(key.project(t));
		TupleSet ts = new TupleSet();
		if(ret != null)
			ts.add(key.reconstruct(t, ret));
		return ts;
	}

	@Override
	public TupleSet lookup(Key key, Tuple t) {
		System.out.println("stasis index refusing to lookup that is not by key!");
		System.exit(-1);
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public TupleSet lookup(Comparable... values) throws BadKeyException {
		return lookup(new Tuple(values));
	}

	@Override
	protected void remove(Tuple t) {
		idx.remove(t);
	}

	@Override
	public String toString() {
		return "stasis index of some sort";
	}

}
