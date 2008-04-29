package p2.types.operator;

import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Index;
import p2.types.table.Key;
import p2.types.table.Schema;

public class HashIndexJoin extends Operator {
	
	private Index index;
	
	private Key lookup;

	public HashIndexJoin(String program, String rule, String id, Index index, Key lookup) {
		super(program, rule, id);
		this.index = index;
		this.lookup = lookup;
	}
	
	@Override
	public TupleSet evaluate(TupleSet tuples) {
		TupleSet result = new SimpleTupleSet();
		
		for (Tuple outer : tuples) {
			TupleSet lookupTuples = index.lookup(lookup.value(outer));
			for (Tuple inner : lookupTuples) {
				result.add(Tuple.join(outer, inner, lookup.attributes()));
			}
		}
		
		return result;
	}

	@Override
	public Schema schema(Schema input) {
		// TODO Auto-generated method stub
		return null;
	}

}
