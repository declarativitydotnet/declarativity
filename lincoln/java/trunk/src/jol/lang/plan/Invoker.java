package jol.lang.plan;

import java.util.Set;

import jol.core.Runtime;
import jol.types.basic.Schema;
import jol.types.exception.PlannerException;
import jol.types.operator.Operator;

public class Invoker extends Term {

	private Expression expr;

	public Invoker(Expression expr) {
		this.expr = expr;
	}

	@Override
	public boolean extension() {
		return true;
	}

	@Override
	public Operator operator(Runtime context, Schema input) throws PlannerException {
		return new jol.types.operator.Invoker(context, this.expr.function(input), program(), rule());
	}

	@Override
	public Set<Variable> requires() {
		return this.expr.variables();
	}

	@Override
	public Schema schema(Schema input) {
		return input;
	}

	@Override
	public String toString() {
		return "Invoker: expression --> " + this.expr.toString();
	}
}
