package lang.ast;



public class Aggregate extends Variable {
	private String function;
	
	public Aggregate(String name, String function, Class type) {
		super(name, type);
		this.function = function;
	}

	public String toString() {
		return this.function + "<" + super.toString() + ">";
	}
}
