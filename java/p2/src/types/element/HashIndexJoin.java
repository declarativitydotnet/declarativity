package types.element;

import types.basic.SimpleTupleSet;
import types.basic.Tuple;
import types.basic.TupleSet;
import types.table.Index;
import types.table.Key;

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
