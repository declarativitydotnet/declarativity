package p2.types.element;

import p2.types.basic.TupleSet;

public class Sort extends Operator {
	
	private Integer[] attributes;

	public Sort(String id, String name, Integer[] attributes) {
		super(id, name);
		this.attributes = attributes;
	}

	@Override
	public TupleSet simple_action(TupleSet t) {
		// TODO Auto-generated method stub
		return null;
	}

}
