package lang.ast;

public class Value<Type> extends Expression {

	private Type value;
	
	public Value(Type value) {
		this.value = value;
	}
	
	public Type value() {
		return this.value;
	}
	
	@Override
	public Class type() {
		return this.value.getClass();
	}
}
