package lang.plan;

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
	
	@Override
	public String toString() {
		if (Integer.class.isAssignableFrom(type()) && 
			value().equals(Integer.MAX_VALUE)) {
			return "infinity";
		}
		return value().toString();
	}
}
