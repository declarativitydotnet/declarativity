package p2.lang.plan;


public class Alias extends Variable {
	
	public Alias(String name, Integer field, Class type) {
		super(name, type);
		this.position(field.intValue());
	}
	
	public String toString() {
		return super.toString() + " := $" + position();
	}
}
