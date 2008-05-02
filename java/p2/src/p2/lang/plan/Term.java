package p2.lang.plan;

import java.util.Set;

public abstract class Term  {
	
	private xtc.tree.Location location;
	
	public void location(xtc.tree.Location location) {
		this.location = location;
	}
	
	public xtc.tree.Location location() {
		return this.location;
	}
	
	@Override
	public abstract String toString();
	
	public abstract Set<Variable> requires();
}
