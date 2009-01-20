package jol.lang.plan;


public class Alias extends Variable {
	
	private int position;
	
	public Alias(String name, int position, Class type) {
		super(name, type);
		this.position = position;
	}
	
	@Override
	public String toString() {
		return super.toString() + " := $" + position;
	}
	
	public int position() {
		return this.position;
	}
}
