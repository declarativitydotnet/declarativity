package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.basic.ValueList;

public class TopK extends Limit {
	
	public TopK(Variable value, Number bottomkConst) {
		super(jol.types.function.Aggregate.TOPK, value, bottomkConst);
	}
	
	public TopK(Variable value, Variable bottomkVar) {
		super(jol.types.function.Aggregate.TOPK, value, bottomkVar);
	}
}
