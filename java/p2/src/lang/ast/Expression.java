package lang.ast;

public abstract class Expression {

	@Override
	public abstract String toString();
	
	/**
	 * @return The java type of the expression value.
	 */
	public abstract Class type();
	
}
