package p2.lang.plan;

import p2.core.Runtime;
import p2.types.exception.UpdateException;

public abstract class Clause implements Comparable<Clause> {
	private xtc.tree.Location location;
	
	protected Clause(xtc.tree.Location location) {
		this.location = location;
	}
	
	public xtc.tree.Location location() {
		return this.location;
	}
	
	public int compareTo(Clause o) {
		return location.compareTo(o.location);
	}

	@Override
	public abstract String toString();
	
	public abstract void set(Runtime context, String program) throws UpdateException;

}
