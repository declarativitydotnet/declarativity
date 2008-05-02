package p2.lang.plan;

import java.util.Set;

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

	@Override
	public Set<Variable> variables() {
		return array.variables();
	}
}
