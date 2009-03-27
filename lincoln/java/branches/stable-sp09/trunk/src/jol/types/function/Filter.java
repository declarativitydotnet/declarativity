package jol.types.function;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;

public class Filter implements TupleFunction<Comparable> {
	public enum Operator{NOT, AND, OR, EQ, NEQ, LTHAN, GTHAN, LEQ, GEQ};
	
	protected Operator oper;
	
	protected TupleFunction<Object> lhs;
	
	protected TupleFunction<Object> rhs;
	
	public Filter(Operator oper, TupleFunction<Object> lhs, TupleFunction<Object> rhs) {
		this.oper = oper;
		this.lhs = lhs;
		this.rhs = rhs;
	}

	public Boolean evaluate(Tuple tuple) throws JolRuntimeException {
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
	private Boolean value(Object c) {
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
	
	private Boolean not(Tuple tuple) throws JolRuntimeException {
		return !value(this.lhs.evaluate(tuple));
	}
	
	private Boolean and(Tuple tuple) throws JolRuntimeException {
		return value(this.lhs.evaluate(tuple)) && value(this.rhs.evaluate(tuple));
	}
	
	private Boolean or(Tuple tuple) throws JolRuntimeException {
		return value(this.lhs.evaluate(tuple)) || value(this.rhs.evaluate(tuple));
	}
	
	private Boolean eq(Tuple tuple) throws JolRuntimeException {
		return this.lhs.evaluate(tuple).equals(this.rhs.evaluate(tuple));
	}
	
	private Boolean neq(Tuple tuple) throws JolRuntimeException {
		return !this.lhs.evaluate(tuple).equals(this.rhs.evaluate(tuple));
	}
	
	private Boolean lthan(Tuple tuple) throws JolRuntimeException {
		Comparable left  = (Comparable) this.lhs.evaluate(tuple);
		Comparable right = (Comparable) this.rhs.evaluate(tuple);
		return left.compareTo(right) < 0;
	}
	
	private Boolean gthan(Tuple tuple) throws JolRuntimeException {
		Comparable left  = (Comparable) this.lhs.evaluate(tuple);
		Comparable right = (Comparable) this.rhs.evaluate(tuple);
		return left.compareTo(right) > 0;
	}
	
	private Boolean leq(Tuple tuple) throws JolRuntimeException {
		Comparable left  = (Comparable) this.lhs.evaluate(tuple);
		Comparable right = (Comparable) this.rhs.evaluate(tuple);
		return left.compareTo(right) <= 0;
	}
	
	private Boolean geq(Tuple tuple) throws JolRuntimeException {
		Comparable left  = (Comparable) this.lhs.evaluate(tuple);
		Comparable right = (Comparable) this.rhs.evaluate(tuple);
		return left.compareTo(right) >= 0;
	}

	public Class returnType() {
		return Boolean.class;
	}
	
}
