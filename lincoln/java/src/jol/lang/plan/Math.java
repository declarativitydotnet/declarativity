package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.function.BasicMath;
import jol.types.function.TupleFunction;


public class Math extends Expression {
	
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
	
	private Expression lhs;
	
	private Expression rhs;
	
	public Math(String oper, Expression lhs, Expression rhs) {
		this.oper = oper;
		this.lhs = lhs;
		this.rhs = rhs;
	}

	@Override
	public Class type() {
		return lhs.type();
	}
	
	@Override
	public String toString() {
		return "(" + lhs + " " + oper + " " + rhs + ")";
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
	public TupleFunction function() {
		return BasicMath.function(oper, lhs.function(), rhs.function());
	}
}
