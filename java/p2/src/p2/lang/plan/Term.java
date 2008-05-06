package p2.lang.plan;

import java.util.Set;

public abstract class Term {
	private static long identifier = 0;
	
	private xtc.tree.Location location;
	
	private String identifer;
	
	protected Term() {
		this.identifer = "Term:" + identifier++;
	}
	
	public void location(xtc.tree.Location location) {
		this.location = location;
	}
	
	public xtc.tree.Location location() {
		return this.location;
	}
	
	public int compareTo(Term o) {
		return this.identifer.compareTo(o.identifer);
	}
	
	@Override
	public abstract String toString();
	
	public abstract Set<Variable> requires();
}
