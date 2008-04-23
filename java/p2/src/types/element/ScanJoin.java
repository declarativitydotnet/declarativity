package types.element;

import types.basic.SimpleTupleSet;
import types.basic.Tuple;
import types.basic.TupleSet;
import types.table.Table;

public class ScanJoin extends Operator {
	
	/* The table to be scanned. */
	private Table table;
	
	private Integer[] outerKey;
	
	private Integer[] innerKey;

	public ScanJoin(String id, String name, Table table, Integer[] outerKey, Integer[] innerKey) {
		super(id, name);
		assert(outerKey.length == innerKey.length);
		this.table = table;
		this.outerKey = outerKey;
		this.innerKey = innerKey;
	}
	
	/**
	 * Determine the join result of the outerSet and the inner table.
	 */
	public TupleSet simple_action(TupleSet outerSet) {
		TupleSet result = new SimpleTupleSet();
		for (Tuple inner : table) {
			for (Tuple outer : outerSet) {
				result.add(Tuple.join(outer, inner, innerKey));
			}
		}
		return result;
	}

}
