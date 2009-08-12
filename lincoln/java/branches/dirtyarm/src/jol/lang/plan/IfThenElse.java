package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import xtc.tree.Node;

import jol.types.basic.Tuple;
import jol.types.basic.Schema;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;

public class IfThenElse extends Expression {
	
	private Class type;
	
	private Boolean<?> ifexpr;
	
	private Expression<?> thenexpr;
	
	private Expression<?> elseexpr;

	public IfThenElse(Node node, Class type, Boolean ifexpr, Expression thenexpr, Expression elseexpr) {
		super(node);
		this.type = type;
		this.ifexpr = ifexpr;
		this.thenexpr = thenexpr;
		this.elseexpr = elseexpr;
	}
	
	public Expression clone() {
		return new IfThenElse(node(), type, (Boolean)ifexpr.clone(), thenexpr.clone(), elseexpr.clone());
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
	public TupleFunction function(Schema schema) throws PlannerException {
		final TupleFunction<java.lang.Boolean> test = ifexpr.function(schema);
		final TupleFunction thencase = thenexpr.function(schema);
		final TupleFunction elsecase = elseexpr.function(schema);
		
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				return test.evaluate(tuple).equals(java.lang.Boolean.TRUE) ? 
						thencase.evaluate(tuple) : elsecase.evaluate(tuple);
			}

			public Class returnType() {
				return type();
			}
		};
	}

}
