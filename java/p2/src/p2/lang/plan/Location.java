package p2.lang.plan;



public class Location extends Variable {
	
	public Location(String name, Class type) {
		super(name, type);
	}
	
	public Location clone() {
		Location clone = new Location(name(), type());
		clone.position(this.position());
		return clone;
	}
	
	public String toString() {
		return "@" + super.toString();
	}
}
