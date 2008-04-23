package lang.ast;

public class Range extends Boolean {
	
	private Variable variable;

	public Range(Variable variable, Expression lhs, Expression rhs) {
		super("in", lhs, rhs);
		this.variable = variable;
	}

}
