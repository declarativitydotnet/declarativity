package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;

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

}
