package jol.types.table;

import java.util.HashMap;
import java.util.Iterator;

import jol.core.Runtime;
import jol.types.basic.Tuple;
import jol.types.basic.TupleSet;
import jol.types.exception.BadKeyException;

public class StasisIndex extends Index {

	HashMap<Tuple, Tuple> idx;
	
	public StasisIndex(Runtime context, Table table, Key key, jol.types.table.Index.Type type) {
		super(context, table, key, type);
		idx = new HashMap<Tuple,Tuple>();
	}

	@Override
	protected void insert(Tuple t) {
		idx.put(key().project(t), key().projectValue(t));
	}

	@Override
	public Iterator<Tuple> iterator() {
		return new Iterator<Tuple>() {
			Iterator<Tuple> ks = idx.keySet().iterator();

			public boolean hasNext() {
				return ks.hasNext();
			}

			public Tuple next() {
				Tuple next = ks.next();
				if(next == null) return null;
				return key().reconstruct(next, idx.get(next));
			}

			public void remove() {
				ks.remove();
			}		
		};
	}

	@Override
	public TupleSet lookupByKey(Tuple key) throws BadKeyException {
		if(key.size() != key().size() && key().size() > 0) {
			throw new BadKeyException("Key had wrong number of columns!");
		}
		Tuple ret = idx.get(key);
		TupleSet ts = new TupleSet();
		if(ret != null)
			ts.add(key().reconstruct(key, ret));
		return ts;
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
