package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.basic.ValueList;

public class BottomK extends Limit {

	public BottomK(String name, Number bottomkConst) {
		super(name, jol.types.function.Aggregate.BOTTOMK, bottomkConst);
	}
	
	public BottomK(String name, Variable bottomkVar) {
		super(name, jol.types.function.Aggregate.BOTTOMK, bottomkVar);
	}
}
