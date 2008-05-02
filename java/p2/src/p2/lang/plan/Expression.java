package p2.lang.plan;

import java.util.Set;

public abstract class Expression {
	
	private xtc.tree.Location location;
	
	public void location(xtc.tree.Location location) {
		this.location = location;
	}
	
	public xtc.tree.Location location() {
		return this.location;
	}

	@Override
	public abstract String toString();
	
	/**
	 * @return The java type of the expression value.
	 */
	public abstract Class type();
	
	public abstract Set<Variable> variables();
	
}
