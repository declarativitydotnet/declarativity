package p2.lang.plan;

public class ArrayIndex extends Expression {
	
	private Expression array;
	
	private Integer index;
	
	public ArrayIndex(Expression array, Integer index) {
		this.array = array;
		this.index = index;
	}
	
	@Override
	public Class type() {
		return array.type().getComponentType();
	}
	
	@Override
	public String toString() {
		return "(" + array.toString() + ")[" + index + "]";
	}
}
