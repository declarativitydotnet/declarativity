package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.basic.ValueList;

public class TopK extends Limit {
	
	public TopK(String name, Number bottomkConst) {
		super(name, jol.types.function.Aggregate.TOPK, bottomkConst);
	}
	
	public TopK(String name, Variable bottomkVar) {
		super(name, jol.types.function.Aggregate.TOPK, bottomkVar);
	}
}
