package jol.lang.plan;

import java.util.Set;

import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;
import jol.types.basic.Schema;

public class Cast<C> extends Expression {

	private Class<C> type;
	
	private Expression<?> expression;
	
	public Cast(Class<C> type, Expression expression) {
		this.type = type;
		this.expression = expression;
	}
	
	public Expression clone() {
		return new Cast(type, expression.clone());
	}
	
	@Override
	public TupleFunction function(Schema schema) throws PlannerException {
		return this.expression.function(schema);
	}

	@Override
	public String toString() {
		return "(" + type + ")" + expression;
	}

	@Override
	public Class<C> type() {
		return type;
	}

	@Override
	public Set<Variable> variables() {
		return this.expression.variables();
	}
	
	public Expression<?> expression() {
		return this.expression;
	}

}
