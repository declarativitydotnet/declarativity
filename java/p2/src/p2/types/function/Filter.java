package p2.types.function;

import java.util.Set;

import p2.lang.plan.Variable;
import p2.types.basic.Tuple;

public class Filter implements TupleFunction<Comparable> {
	public enum Operator{NOT, AND, OR, EQ, NEQ, LTHAN, GTHAN, LEQ, GEQ};
	
	private Operator oper;
	
	private TupleFunction<Comparable> lhs;
	
	private TupleFunction<Comparable> rhs;
	
	public Filter(Operator oper, TupleFunction<Comparable> lhs, TupleFunction<Comparable> rhs) {
		this.oper = oper;
		this.lhs = lhs;
		this.rhs = rhs;
	}

	public Boolean evaluate(Tuple tuple) {
		switch (oper) {
		case NOT:   return not(tuple);
		case AND:   return and(tuple);
		case OR:    return or(tuple);
		case EQ:    return eq(tuple);
		case NEQ:   return neq(tuple);
		case LTHAN: return lthan(tuple);
		case GTHAN: return gthan(tuple);
		case LEQ:   return leq(tuple);
		case GEQ:   return geq(tuple);
		}
		// TODO log fatal error.
		return Boolean.FALSE;
	}
	
	/**
	 * Indicates the boolean value of the comparable object.
	 * @param c The comparable object.
	 * @return The boolean value of the comparable object.
	 */
	private Boolean value(Comparable c) {
		if (c == null) {
			return Boolean.FALSE;
		}
		else if (c instanceof Boolean && !((Boolean)c).booleanValue()) {
		    return Boolean.FALSE;	
		}
		else if (c instanceof Number && ((Number)c).intValue() == 0) {
			return Boolean.FALSE;
		}
		else if (c instanceof String && ((String)c) == "") {
			return Boolean.FALSE;
		}
		return Boolean.TRUE;
	}
	
	private Boolean not(Tuple tuple) {
		return !value(this.lhs.evaluate(tuple));
	}
	
	private Boolean and(Tuple tuple) {
		return value(this.lhs.evaluate(tuple)) && value(this.rhs.evaluate(tuple));
	}
	
	private Boolean or(Tuple tuple) {
		return value(this.lhs.evaluate(tuple)) || value(this.rhs.evaluate(tuple));
	}
	
	private Boolean eq(Tuple tuple) {
		return this.lhs.evaluate(tuple).compareTo(this.rhs.evaluate(tuple)) == 0;
	}
	
	private Boolean neq(Tuple tuple) {
		return this.lhs.evaluate(tuple).compareTo(this.rhs.evaluate(tuple)) != 0;
	}
	
	private Boolean lthan(Tuple tuple) {
		return this.lhs.evaluate(tuple).compareTo(this.rhs.evaluate(tuple)) < 0;
	}
	
	private Boolean gthan(Tuple tuple) {
		return this.lhs.evaluate(tuple).compareTo(this.rhs.evaluate(tuple)) > 0;
	}
	
	private Boolean leq(Tuple tuple) {
		return this.lhs.evaluate(tuple).compareTo(this.rhs.evaluate(tuple)) <= 0;
	}
	
	private Boolean geq(Tuple tuple) {
		return this.lhs.evaluate(tuple).compareTo(this.rhs.evaluate(tuple)) >= 0;
	}

	public Class returnType() {
		return Boolean.class;
	}
	
}
