package jol.lang.plan;

import jol.types.basic.ValueList;

public class TopK extends Aggregate {
	
	private Integer topk;

	public TopK(String name, Integer topk) {
		super(name, jol.types.function.Aggregate.TOPK, ValueList.class);
		this.topk = topk;
	}
	
	public Integer k() {
		return this.topk;
	}
}
