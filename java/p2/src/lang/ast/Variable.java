package lang.ast;

public class Variable extends Expression {

	private String name;
	
	private Class type;
	
	public Variable(String name, Class type) {
		this.name = name;
		this.type = type;
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
