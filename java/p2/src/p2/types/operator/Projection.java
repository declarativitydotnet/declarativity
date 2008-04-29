package p2.types.operator;

import java.util.HashSet;

import p2.types.basic.OrderedTupleSet;
import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Schema;


public class Projection extends Operator {
	
	private String tupleName;
	
	private Integer[] attributes;

	public Projection(String program, String rule, String id, String tupleName, Integer[] attributes) {
		super(program, rule, id);
		this.tupleName = tupleName;
		this.attributes = attributes;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) {
		TupleSet result = new SimpleTupleSet();
		
		/* The order is preserved iff the sort attribute is
		 * contained in the projection. */
		if (tuples.sort() != null) {
			result = new OrderedTupleSet(tuples.sort());
			for (Integer sort : tuples.sort()) {
				boolean found = false;
				for (Integer projection : attributes) {
					if (sort.equals(projection)) {
						found = true;
						break;
					}
				}
				if (!found) {
					result = new SimpleTupleSet();
					break;
				}
			}
		}
		
		for (Tuple t : tuples) {
			result.add(new Tuple(tupleName, Tuple.project(t, attributes)));
		}
		return result;
	}

	@Override
	public Schema schema(Schema input) {
		// TODO Auto-generated method stub
		return null;
	}

}
