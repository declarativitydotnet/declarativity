package p2.types.element;

import java.util.HashSet;

import p2.types.basic.OrderedTupleSet;
import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;


public class Projection extends Operator {
	
	private String tupleName;
	
	private Integer[] attributes;

	public Projection(String id, String name, String tupleName, Integer[] attributes) {
		super(id, name);
		this.tupleName = tupleName;
		this.attributes = attributes;
	}

	@Override
	public TupleSet simple_action(TupleSet tuples) {
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

}
