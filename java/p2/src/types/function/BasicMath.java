package types.function;

import types.basic.Function;
import types.basic.Tuple;
import types.exception.TypeException;

public class BasicMath implements Function<Comparable> {
	public enum Operator{UPLUS, UMINUS, PLUS, MINUS, TIMES, DIVIDE, MOD, POW, LSHIFT, RSHIFT};
	
	private Operator oper;
	
	private Function<Comparable> lhs;
	
	private Function<Comparable> rhs;
	
	public BasicMath(Operator oper, Function<Comparable> lhs, Function<Comparable> rhs) {
		this.oper = oper;
		this.lhs = lhs;
		this.rhs = rhs;
	}

	public Comparable eval(Tuple tuple) {
		Comparable lhs = this.lhs.eval(tuple);
		Comparable rhs = this.rhs != null ? this.rhs.eval(tuple) : null;
		
		try {
			if (lhs instanceof Double) {
				return doubleEval((Number)lhs, (Number)rhs);
			}
			else if (lhs instanceof Float) {
				return floatEval((Number)lhs, (Number)rhs);
			}
			else if (lhs instanceof Integer) {
				return intEval((Number)lhs, (Number)rhs);
			}
			else if (lhs instanceof Long) {
				return longEval((Number)lhs, (Number)rhs);
			}
			else if (lhs instanceof Short) {
				return shortEval((Number)lhs, (Number)rhs);
			}
			else if (lhs instanceof String) {
				return stringEval((String)lhs, rhs);
			}
		} catch(TypeException e) {
			e.printStackTrace();
			// TODO Fatal error.
		}
		// TODO Fatal error.
		return null;
	}
	
	public Double doubleEval(Number lhs, Number rhs) throws TypeException {
		switch(oper) {
		case UPLUS:  return new Double(lhs.doubleValue() + 1D);
		case UMINUS: return new Double(lhs.doubleValue() - 1D);
		case PLUS:   return new Double(lhs.doubleValue() + rhs.doubleValue());
		case MINUS:  return new Double(lhs.doubleValue() - rhs.doubleValue());
		case TIMES:  return new Double(lhs.doubleValue() * rhs.doubleValue());
		case DIVIDE: return new Double(lhs.doubleValue() / rhs.doubleValue());
		case MOD:    return new Double(lhs.doubleValue() % rhs.doubleValue());
		case POW:    return new Double(java.lang.Math.pow(lhs.doubleValue(),rhs.doubleValue()));
		default: throw new TypeException("Operator " + oper + " undefined over type!", Double.TYPE);
		}
	}
	
	public Float floatEval(Number lhs, Number rhs) throws TypeException {
		switch(oper) {
		case UPLUS:  return new Float(lhs.floatValue() + 1F);
		case UMINUS: return new Float(lhs.floatValue() - 1F);
		case PLUS:   return new Float(lhs.floatValue() + rhs.floatValue());
		case MINUS:  return new Float(lhs.floatValue() - rhs.floatValue());
		case TIMES:  return new Float(lhs.floatValue() * rhs.floatValue());
		case DIVIDE: return new Float(lhs.floatValue() / rhs.floatValue());
		case MOD:    return new Float(lhs.floatValue() % rhs.floatValue());
		case POW:    return new Float(java.lang.Math.pow(lhs.floatValue(),rhs.floatValue()));
		default: throw new TypeException("Operator " + oper + " undefined over type!", Float.TYPE);
		}
	}
	
	public Integer intEval(Number lhs, Number rhs) throws TypeException {
		switch(oper) {
		case UPLUS:  return new Integer(lhs.intValue() + 1);
		case UMINUS: return new Integer(lhs.intValue() - 1);
		case PLUS:   return new Integer(lhs.intValue() + rhs.intValue());
		case MINUS:  return new Integer(lhs.intValue() - rhs.intValue());
		case TIMES:  return new Integer(lhs.intValue() * rhs.intValue());
		case DIVIDE: return new Integer(lhs.intValue() / rhs.intValue());
		case MOD:    return new Integer(lhs.intValue() % rhs.intValue());
		case POW:    return new Integer((int)java.lang.Math.pow(lhs.intValue(),rhs.intValue()));
		case LSHIFT: return new Integer(lhs.intValue() << rhs.intValue());
		case RSHIFT: return new Integer(lhs.intValue() >> rhs.intValue());
		default: throw new TypeException("Unknown operator " + oper + " undefined over type!", Integer.TYPE);
		}
	}
	
	public Long longEval(Number lhs, Number rhs) throws TypeException {
		switch(oper) {
		case UPLUS:  return new Long(lhs.longValue() + 1L);
		case UMINUS: return new Long(lhs.longValue() - 1L);
		case PLUS:   return new Long(lhs.longValue() + rhs.longValue());
		case MINUS:  return new Long(lhs.longValue() - rhs.longValue());
		case TIMES:  return new Long(lhs.longValue() * rhs.longValue());
		case DIVIDE: return new Long(lhs.longValue() / rhs.longValue());
		case MOD:    return new Long(lhs.longValue() % rhs.longValue());
		case POW:    return new Long((long)java.lang.Math.pow(lhs.longValue(),rhs.longValue()));
		case LSHIFT: return new Long(lhs.longValue() << rhs.longValue());
		case RSHIFT: return new Long(lhs.longValue() >> rhs.longValue());
		default: throw new TypeException("Operator " + oper + " undefined over type!", Long.TYPE);
		}
	}
	
	public Short shortEval(Number lhs, Number rhs) throws TypeException {
		switch(oper) {
		case UPLUS:  return new Short((short) (lhs.shortValue() + (short) 1));
		case UMINUS: return new Short((short) (lhs.shortValue() - (short) 1));
		case PLUS:   return new Short((short) (lhs.shortValue() + rhs.shortValue()));
		case MINUS:  return new Short((short) (lhs.shortValue() - rhs.shortValue()));
		case TIMES:  return new Short((short) (lhs.shortValue() * rhs.shortValue()));
		case DIVIDE: return new Short((short) (lhs.shortValue() / rhs.shortValue()));
		case MOD:    return new Short((short) (lhs.shortValue() % rhs.shortValue()));
		case POW:    return new Short((short)java.lang.Math.pow(lhs.shortValue(),rhs.shortValue()));
		default: throw new TypeException("Operator " + oper + " undefined over type!", Short.TYPE);
		}
	}
	
	public String stringEval(String lhs, Comparable rhs) throws TypeException {
		switch(oper) {
		case PLUS:   return new String(lhs.toString() + rhs.toString());
		default: throw new TypeException("Operator " + oper + " undefined over type!", Short.TYPE);
		}
	}
}
