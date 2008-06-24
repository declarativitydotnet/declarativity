package p2.lang.plan;

import java.util.Set;

import p2.types.function.TupleFunction;

public class Cast extends Expression {

	private Class type;
	
	private Expression expression;
	
	public Cast(Class type, Expression expression) {
		this.type = type;
		this.expression = expression;
	}
	
	@Override
	public TupleFunction function() {
		return this.expression.function();
	}

	@Override
	public String toString() {
		return "(" + type + ")" + expression;
	}

	@Override
	public Class type() {
		return type;
	}

	@Override
	public Set<Variable> variables() {
		return this.expression.variables();
	}
	
	public Expression expression() {
		return this.expression;
	}

}
