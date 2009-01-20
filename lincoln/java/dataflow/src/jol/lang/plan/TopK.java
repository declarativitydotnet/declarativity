package jol.lang.plan;

public class TopK extends Limit {
	
	public TopK(Variable value, Number bottomkConst) {
		super(jol.types.function.Aggregate.TOPK, value, bottomkConst);
	}
	
	public TopK(Variable value, Variable bottomkVar) {
		super(jol.types.function.Aggregate.TOPK, value, bottomkVar);
	}
	
	private TopK(TopK copy) {
		super(copy);
	}
	
	@Override
	public Expression clone() {
		return new TopK(this);
	}
}
