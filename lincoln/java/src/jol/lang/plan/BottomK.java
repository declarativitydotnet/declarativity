package jol.lang.plan;

public class BottomK extends Limit {

	public BottomK(Variable value, Number bottomkConst) {
		super(jol.types.function.Aggregate.BOTTOMK, value, bottomkConst);
	}
	
	public BottomK(Variable value, Variable bottomkVar) {
		super(jol.types.function.Aggregate.BOTTOMK, value, bottomkVar);
	}
}
