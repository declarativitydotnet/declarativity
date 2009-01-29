package jol.lang.plan;

import xtc.tree.Node;

public class Null extends Value<Object> {
	
	public Null(Node node) {
		super(node, null);
	}
	
	@Override
	public String toString() {
		return "null";
	}
}
