package lang.plan;



public class Location extends Variable {
	
	public Location(String name, Class type) {
		super(name, type);
	}
	
	public String toString() {
		return "@" + super.toString();
	}
}
