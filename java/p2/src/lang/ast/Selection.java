package lang.ast;

public class Selection extends Term {
	
	private Expression predicate;
	
	public Selection(Expression predicate) {
		this.predicate = predicate;
		assert(predicate.type() == java.lang.Boolean.class);
	}
	
	@Override
	public String toString() {
		return predicate.toString();
	}
}
