package jol.lang.plan;

import jol.types.basic.ValueList;

public class BottomK extends Aggregate {
	private Integer bottomk;

	public BottomK(String name, Integer bottomk) {
		super(name, jol.types.function.Aggregate.BOTTOMK, ValueList.class);
		this.bottomk = bottomk;
	}
	
	public Integer k() {
		return this.bottomk;
	}
}
