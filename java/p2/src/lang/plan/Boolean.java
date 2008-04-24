package lang.plan;

import java.util.HashMap;
import java.util.Map;

import types.basic.Function;

public class Boolean extends Expression {
	private static final Map<String, Function> Operator = new HashMap<String, Function>();

	public final static String AND     = "&&";
	public final static String OR      = "||";
	public final static String NOT     = "!";
	public final static String EQUAL   = "==";
	public final static String NEQUAL  = "!=";
	public final static String LEQUAL  = "<=";
	public final static String GEQUAL  = ">=";
	public final static String LESS    = "<";
	public final static String GREATER = ">";
	public final static String IN      = "in";
	
	static {
		Operator.put(AND,     null);
		Operator.put(OR,      null);
		Operator.put(NOT,     null);
		Operator.put(EQUAL,   null);
		Operator.put(NEQUAL,  null);
		Operator.put(LEQUAL,  null);
		Operator.put(GEQUAL,  null);
		Operator.put(LESS,    null);
		Operator.put(GREATER, null);
		Operator.put(IN,      null);
	}
	
	private String oper;
	
	private Expression lhs;
	
	private Expression rhs;
	
	public Boolean(String oper, Expression lhs, Expression rhs) {
		this.oper = oper;
		this.lhs = lhs;
		this.rhs = rhs;
	}

	@Override
	public Class type() {
		return java.lang.Boolean.class;
	}
	
	@Override
	public String toString() {
		return "(" + lhs.toString() + " " + 
		      oper + " " + rhs.toString() + ")";
	}
}
