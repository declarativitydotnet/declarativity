package jol.lang.plan;

import xtc.tree.Node;

public class BottomK extends Limit {

	public BottomK(Node node, Variable value, Number bottomkConst) {
		super(node, jol.types.function.Aggregate.BOTTOMK, value, bottomkConst);
	}
	
	public BottomK(Node node, Variable value, Variable bottomkVar) {
		super(node, jol.types.function.Aggregate.BOTTOMK, value, bottomkVar);
	}
	
	private BottomK(BottomK copy) {
		super(copy);
	}
	
	@Override
	public Expression clone() {
		return new BottomK(this);
	}
}
