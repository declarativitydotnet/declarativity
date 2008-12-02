package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.exception.PlannerException;
import jol.types.function.BasicMath;
import jol.types.function.TupleFunction;


public class Math<N extends java.lang.Number> extends Expression<N> {
	
	public final static String ADD    = "+";
	public final static String INC    = "++";
	public final static String MINUS  = "-";
	public final static String DEC    = "--";
	public final static String TIMES  = "*";
	public final static String DIVIDE = "/";
	public final static String MOD    = "%";
	public final static String POW    = "^";
	public final static String LSHIFT = "<<";
	public final static String RSHIFT = ">>";


	
	private String oper;
	
	private Expression<N> lhs;
	
	private Expression<N> rhs;
	
	public Math(String oper, Expression<N> lhs, Expression<N> rhs) {
		this.oper = oper;
		this.lhs = lhs;
		this.rhs = rhs;
	}
	
	public Expression clone() {
		return new Math(oper, lhs.clone(), rhs.clone());
	}

	@Override
	public Class<N> type() {
		return lhs.type();
	}
	
	@Override
	public String toString() {
		return "MATH(" + lhs + " " + oper + " " + rhs + ")";
	}

	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		variables.addAll(lhs.variables());
		if (rhs != null) {
			variables.addAll(rhs.variables());
		}
		return variables;
	}

	@Override
	public TupleFunction<N> function() {
		try {
			return BasicMath.function(oper, lhs.function(), rhs.function());
		} catch (PlannerException e) {
			e.printStackTrace();
			return null;
		}
	}
}
