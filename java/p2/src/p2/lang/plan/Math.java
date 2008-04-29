package p2.lang.plan;

import java.util.HashMap;
import java.util.Map;

import p2.types.function.TupleFunction;


public class Math extends Expression {
	private static final Map<String, TupleFunction> Operator = new HashMap<String, TupleFunction>();
	
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

	static {
		Operator.put(ADD,    null);
		Operator.put(INC,    null);
		Operator.put(MINUS,  null);
		Operator.put(DEC,    null);
		Operator.put(TIMES,  null);
		Operator.put(DIVIDE, null);
		Operator.put(MOD,    null);
		Operator.put(POW,    null);
		Operator.put(LSHIFT, null);
		Operator.put(RSHIFT, null);
	}
	
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
}
