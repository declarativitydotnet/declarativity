package jol.lang.plan;

public abstract class Reference extends Expression {
	
	protected Class type;
	
	protected String name;

	public Reference(Class type, String name) {
		this.type = type;
		this.name = name;
	}
	
	@Override
	public String toString() {
		return this.name;
	}

	@Override
	public Class type() {
		return this.type;
	}
	
	public abstract Expression object();
}
