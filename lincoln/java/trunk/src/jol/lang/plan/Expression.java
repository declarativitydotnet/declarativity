package jol.lang.plan;

import java.util.Set;

import jol.types.function.TupleFunction;

public abstract class Expression<C> {
	
	private xtc.tree.Location location;
	
	private int position;
	
	public void location(xtc.tree.Location location) {
		this.location = location;
	}
	
	public xtc.tree.Location location() {
		return this.location;
	}

	@Override
	public abstract String toString();
	
	public int position() {
		return this.position;
	}
	
	public void position(int position) {
		this.position = position;
	}
	
	@Override
	public abstract Expression clone();
	
	/**
	 * @return The java type of the expression value.
	 */
	public abstract Class<C> type();
	
	public abstract Set<Variable> variables();
	
	public abstract TupleFunction<C> function();
	
}
