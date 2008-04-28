package p2.lang.plan;

public class Range extends Expression {
	
	public enum Operator {CC, // [] 
		                  OC, // (]
		                  CO, // [)
		                  OO  // () 
		                 };
	
	private Operator oper;
	
	private Expression begin;
	
	private Expression end;

	public Range(Operator oper, Expression begin, Expression end) {
		this.oper = oper;
		this.begin = begin;
		this.end = end;
	}

	@Override
	public Class type() {
		return begin.type();
	}
	
	@Override
	public String toString() {
		if (oper == Operator.CC)
			return "[" + begin + ", " + end + "]";
		if (oper == Operator.OC)
			return "(" + begin + ", " + end + "]";
		if (oper == Operator.CO)
			return "[" + begin + ", " + end + ")";
		if (oper == Operator.OO)
			return "(" + begin + ", " + end + ")";
		assert(false);
		return "RANGE ERROR";
	}

}
