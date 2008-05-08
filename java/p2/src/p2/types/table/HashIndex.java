package p2.types.table;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Key.Value;

public class HashIndex extends Index {
	
	private Map<Key.Value, TupleSet> map;

	public HashIndex(Table table, Key key, Type type) {
		super(table, key, type);
		map = new HashMap<Key.Value, TupleSet>();
	}
	
	@Override
	public String toString() {
		String out = "Index " + table().name() + "\n";
		for (Tuple tuple : this) {
			out += tuple + "\n";
		}
		return out;
	}

	@Override
	public void insert(Tuple t) {
		Key.Value key = key().value(t);
		if (this.map.containsKey(key)) {
			this.map.get(key).add(t);
		}
		else {
			TupleSet tuples = new TupleSet(table().name());
			tuples.add(t);
			this.map.put(key, tuples);
		}
	}

	@Override
	public TupleSet lookup(Tuple t) {
		return lookup(key().value(t));
	}

	@Override
	public TupleSet lookup(Key.Value key) {
		if (this.map.containsKey(key)) {
			return this.map.get(key);
		}
		return new TupleSet(table().name());
	}

	@Override
	public void remove(Tuple t) {
		Key.Value key = key().value(t);
		if (this.map.containsKey(key)) {
			this.map.get(key).remove(t);
		}
	}

	@Override
	public Iterator<Tuple> iterator() {
		Set<Tuple> tuples = new HashSet<Tuple>();
		for (TupleSet set : this.map.values()) {
			tuples.addAll(set);
		}
		return tuples.iterator();
	}
}
