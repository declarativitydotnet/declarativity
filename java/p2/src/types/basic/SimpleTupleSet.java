package types.basic;

import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;


public class SimpleTupleSet extends TupleSet {

	public SimpleTupleSet() {
		super(new HashSet<Tuple>(), null);
	}
	
	public SimpleTupleSet(Collection<Tuple> c) {
		super(new HashSet<Tuple>(c), null);
	}


}
