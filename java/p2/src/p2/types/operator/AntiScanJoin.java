package p2.types.operator;

import p2.lang.plan.Predicate;
import p2.types.basic.TupleSet;

public class AntiScanJoin extends Join {

	public AntiScanJoin(Predicate predicate) {
		super(predicate);
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String toString() {
		// TODO Auto-generated method stub
		return null;
	}

}
