package lang.plan;

public class Variable extends Expression {
	public final static String DONTCARE = "_";

	private String name;
	
	private Class type;
	
	public Variable(String name, Class type) {
		this.name = name;
		this.type = type;
	}
	
	public static Variable dontCare(Class type) {
		return new Variable(DONTCARE, type);
	}
	
	public boolean isDontCare() {
		return DONTCARE.equals(this.name);
	}
	
	@Override
	public String toString() {
		return name();
	}
	
	public String name() {
		return this.name;
	}

	@Override
	public Class type() {
		return this.type;
	}
	
	public void type(Class type) {
		this.type = type;
	}
	
}
