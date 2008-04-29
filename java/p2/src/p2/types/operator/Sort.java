package p2.types.operator;

import p2.types.basic.TupleSet;
import p2.types.table.Schema;

public class Sort extends Operator {
	
	private Integer[] attributes;

	public Sort(String program, String rule, String id, Integer[] attributes) {
		super(program, rule, id);
		this.attributes = attributes;
	}

	@Override
	public TupleSet evaluate(TupleSet t) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Schema schema(Schema input) {
		return input;
	}

}
