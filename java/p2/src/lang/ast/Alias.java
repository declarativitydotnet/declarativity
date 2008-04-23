package lang.ast;


public class Alias extends Variable {
	
	private Integer field;

	public Alias(String name, Integer field, Class type) {
		super(name, type);
		this.field = field;
	}
	
	public Integer field() {
		return this.field;
	}
}
