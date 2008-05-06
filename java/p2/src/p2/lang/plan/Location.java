package p2.lang.plan;



public class Location extends Variable {
	
	public Location(String name, Class type) {
		super(name, type);
	}
	
	public Location clone() {
		return new Location(name(), type());
	}
	
	public String toString() {
		return "@" + super.toString();
	}
}
