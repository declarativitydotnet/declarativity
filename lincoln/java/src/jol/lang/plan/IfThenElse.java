package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.exception.P2RuntimeException;
import jol.types.function.TupleFunction;

public class IfThenElse extends Expression {
	
	private Class type;
	
	private Boolean ifexpr;
	
	private Expression thenexpr;
	
	private Expression elseexpr;

	public IfThenElse(Class type, Boolean ifexpr, Expression thenexpr, Expression elseexpr) {
		this.type = type;
		this.ifexpr = ifexpr;
		this.thenexpr = thenexpr;
		this.elseexpr = elseexpr;
	}

	@Override
	public String toString() {
		return ifexpr.toString() + " ? " + 
		       thenexpr.toString() + " : " + 
		       elseexpr.toString();
	}

	@Override
	public Class type() {
		return this.type;
	}

	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		variables.addAll(ifexpr.variables());
		variables.addAll(thenexpr.variables());
		variables.addAll(elseexpr.variables());
		return variables;
	}

	@Override
	public TupleFunction function() {
		return new TupleFunction() {
			private final TupleFunction<java.lang.Boolean> test = ifexpr.function();
			private final TupleFunction thencase = thenexpr.function();
			private final TupleFunction elsecase = elseexpr.function();
			public Object evaluate(Tuple tuple) throws P2RuntimeException {
				return test.evaluate(tuple).equals(java.lang.Boolean.TRUE) ? 
						thencase.evaluate(tuple) : elsecase.evaluate(tuple);
			}

			public Class returnType() {
				return type();
			}
		};
	}

}
