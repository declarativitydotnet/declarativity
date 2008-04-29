package p2.types.operator;

import p2.types.basic.SimpleTupleSet;
import p2.types.basic.Tuple;
import p2.types.basic.TupleSet;
import p2.types.table.Schema;
import p2.types.table.Table;

public class ScanJoin extends Operator {
	
	/* The table to be scanned. */
	private Table table;
	
	private Integer[] outerKey;
	
	private Integer[] innerKey;

	public ScanJoin(String program, String rule, String id, Table table, Integer[] outerKey, Integer[] innerKey) {
		super(program, rule, id);
		assert(outerKey.length == innerKey.length);
		this.table = table;
		this.outerKey = outerKey;
		this.innerKey = innerKey;
	}
	
	@Override
	public TupleSet evaluate(TupleSet outerSet) {
		TupleSet result = new SimpleTupleSet();
		for (Tuple inner : table) {
			for (Tuple outer : outerSet) {
				result.add(Tuple.join(outer, inner, innerKey));
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
