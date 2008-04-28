package p2.types.element;

import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Index;
import p2.types.table.Key;

public class HashIndexJoin extends Operator {
	
	private Index index;
	
	private Key lookup;

	public HashIndexJoin(String id, String name, Index index, Key lookup) {
		super(id, name);
		this.index = index;
		this.lookup = lookup;
	}
	
	@Override
	public TupleSet simple_action(TupleSet tuples) {
		TupleSet result = new SimpleTupleSet();
		
		for (Tuple outer : tuples) {
			TupleSet lookupTuples = index.lookup(lookup.value(outer));
			for (Tuple inner : lookupTuples) {
				result.add(Tuple.join(outer, inner, lookup.attributes()));
			}
		}
		
		return result;
	}

}
