package lang.plan;

public abstract class Clause {
	private xtc.tree.Location location;
	
	public void location(xtc.tree.Location location) {
		this.location = location;
	}
	
	public xtc.tree.Location location() {
		return this.location;
	}

	@Override
	public abstract String toString();
}
