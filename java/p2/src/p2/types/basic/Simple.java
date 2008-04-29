package p2.types.basic;

import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;

import p2.types.table.Schema;


public class Simple extends TupleSet {

	public Simple(String name, Schema schema) {
		super(name, schema, new HashSet<Tuple>());
	}
	
	public Simple(String name, Schema schema, Collection<Tuple> c) {
		super(name, schema, new HashSet<Tuple>(c));
	}
}
