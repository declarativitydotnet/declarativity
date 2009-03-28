package jol.lang.plan;

import xtc.tree.Node;

public class TopK extends Limit {
	
	public TopK(Node node, Variable value, Number bottomkConst) {
		super(node, jol.types.function.Aggregate.TOPK, value, bottomkConst);
	}
	
	public TopK(Node node, Variable value, Variable bottomkVar) {
		super(node, jol.types.function.Aggregate.TOPK, value, bottomkVar);
	}
	
	private TopK(TopK copy) {
		super(copy);
	}
	
	@Override
	public Expression clone() {
		return new TopK(this);
	}
}
