package jol.lang.plan;

import xtc.tree.Node;

public abstract class Reference extends Expression {
	
	protected Class type;
	
	protected String name;

	public Reference(Node node, Class type, String name) {
		super(node);
		this.type = type;
		this.name = name;
	}
	
	@Override
	public String toString() {
		return this.name;
	}

	@Override
	public Class type() {
		return this.type;
	}
	
	public abstract Expression object();
}
