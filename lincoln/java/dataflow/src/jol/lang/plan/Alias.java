package jol.lang.plan;

import xtc.tree.Node;


public class Alias extends Variable {
	
	private int position;
	
	public Alias(Node node, String name, int position, Class type) {
		super(node, name, type);
		this.position = position;
	}
	
	@Override
	public String toString() {
		return super.toString() + " := $" + position;
	}
	
	public int position() {
		return this.position;
	}
}
