package jol.types.function;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import jol.lang.parse.TypeChecker;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;

public class BasicMath {
	public enum Operator{UPLUS, UMINUS, PLUS, MINUS, TIMES, DIVIDE, MOD, POW, LSHIFT, RSHIFT};

	private final static Map<String, Operator> operators = new HashMap<String, Operator>();
	static {
		operators.put("++", Operator.UPLUS);
		operators.put("--", Operator.UMINUS);
		operators.put("+",  Operator.PLUS);
		operators.put("-",  Operator.MINUS);
		operators.put("*",  Operator.TIMES);
		operators.put("/",  Operator.DIVIDE);
		operators.put("%",  Operator.MOD);
		operators.put("^",  Operator.POW);
		operators.put("<<", Operator.LSHIFT);
		operators.put(">>", Operator.RSHIFT);
	}


	public static TupleFunction function(String oper, TupleFunction lhs, TupleFunction rhs) throws PlannerException {
		Class type = TypeChecker.type(lhs.returnType().getSimpleName());
		if (Double.class.isAssignableFrom(type)) {
			return new DoubleEval(operators.get(oper), lhs, rhs);
		}
		if (Float.class.isAssignableFrom(type)) {
			return new FloatEval(operators.get(oper), lhs, rhs);
		}
		if (Integer.class.isAssignableFrom(type)) {
			return new IntegerEval(operators.get(oper), lhs, rhs);
		}
		if (String.class.isAssignableFrom(type)) {
			return new StringEval(operators.get(oper), lhs, rhs);
		}
		if (Short.class.isAssignableFrom(type)) {
			return new ShortEval(operators.get(oper), lhs, rhs);
		}
		if (Set.class.isAssignableFrom(type)) {
			return new SetEval(operators.get(oper), lhs, rhs);
		}
		if (Long.class.isAssignableFrom(type)) {
			return new LongEval(operators.get(oper), lhs, rhs);
		}
		throw new PlannerException("Unknown type " + type);
	}

	public static class DoubleEval implements TupleFunction<java.lang.Double> {
		private Operator oper;
		private TupleFunction lhs;
		private TupleFunction rhs;

		DoubleEval(Operator oper, TupleFunction lhs, TupleFunction rhs) {
			this.oper = oper;
			this.lhs = lhs;
			this.rhs = rhs;
		}

		public java.lang.Double evaluate(Tuple tuple) throws JolRuntimeException {
			Number lhs = (Number) this.lhs.evaluate(tuple);
			Number rhs = this.rhs == null ? null : (Number) this.rhs.evaluate(tuple);
			switch(oper) {
			case UPLUS:  return new Double(lhs.doubleValue() + 1D);
			case UMINUS: return new Double(lhs.doubleValue() - 1D);
			case PLUS:   return new Double(lhs.doubleValue() + rhs.doubleValue());
			case MINUS:  return new Double(lhs.doubleValue() - rhs.doubleValue());
			case TIMES:  return new Double(lhs.doubleValue() * rhs.doubleValue());
			case DIVIDE: return new Double(lhs.doubleValue() / rhs.doubleValue());
			case MOD:    return new Double(lhs.doubleValue() % rhs.doubleValue());
			case POW:    return new Double(java.lang.Math.pow(lhs.doubleValue(),rhs.doubleValue()));
			default: throw new JolRuntimeException("UNKNOWN MATH OPERATION " + oper);
			}
		}

		public Class returnType() {
			return Double.class;
		}
	}

	public static class FloatEval implements TupleFunction {
		private Operator oper;
		private TupleFunction lhs;
		private TupleFunction rhs;

		FloatEval(Operator oper, TupleFunction lhs, TupleFunction rhs) {
			this.oper = oper;
			this.lhs = lhs;
			this.rhs = rhs;
		}

		public Object evaluate(Tuple tuple) throws JolRuntimeException {
			Number lhs = (Number) this.lhs.evaluate(tuple);
			Number rhs = this.rhs == null ? null : (Number) this.rhs.evaluate(tuple);
			switch(oper) {
			case UPLUS:  return new Float(lhs.floatValue() + 1F);
			case UMINUS: return new Float(lhs.floatValue() - 1F);
			case PLUS:   return new Float(lhs.floatValue() + rhs.floatValue());
			case MINUS:  return new Float(lhs.floatValue() - rhs.floatValue());
			case TIMES:  return new Float(lhs.floatValue() * rhs.floatValue());
			case DIVIDE: return new Float(lhs.floatValue() / rhs.floatValue());
			case MOD:    return new Float(lhs.floatValue() % rhs.floatValue());
			case POW:    return new Float(java.lang.Math.pow(lhs.floatValue(),rhs.floatValue()));
			default: throw new JolRuntimeException("UNKNOWN MATH OPERATION " + oper);
			}
		}

		public Class returnType() {
			return Float.class;
		}
	}

	public static class IntegerEval implements TupleFunction {
		private Operator oper;
		private TupleFunction lhs;
		private TupleFunction rhs;

		IntegerEval(Operator oper, TupleFunction lhs, TupleFunction rhs) {
			this.oper = oper;
			this.lhs = lhs;
			this.rhs = rhs;
		}

		public Object evaluate(Tuple tuple) throws JolRuntimeException {
			Number lhs = (Number) this.lhs.evaluate(tuple);
			Number rhs = this.rhs == null ? null : (Number) this.rhs.evaluate(tuple);
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
			default: throw new JolRuntimeException("UNKNOWN MATH OPERATION " + oper);
			}
		}

		public Class returnType() {
			return Integer.class;
		}
	}

	public static class LongEval implements TupleFunction {
		private Operator oper;
		private TupleFunction lhs;
		private TupleFunction rhs;

		LongEval(Operator oper, TupleFunction lhs, TupleFunction rhs) {
			this.oper = oper;
			this.lhs = lhs;
			this.rhs = rhs;
		}

		public Object evaluate(Tuple tuple) throws JolRuntimeException {
			Number lhs = (Number) this.lhs.evaluate(tuple);
			Number rhs = this.rhs == null ? null : (Number) this.rhs.evaluate(tuple);
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
			default: throw new JolRuntimeException("UNKNOWN MATH OPERATION " + oper);
			}
		}

		public Class returnType() {
			return Long.class;
		}
	}

	public static class ShortEval implements TupleFunction {
		private Operator oper;
		private TupleFunction lhs;
		private TupleFunction rhs;

		ShortEval(Operator oper, TupleFunction lhs, TupleFunction rhs) {
			this.oper = oper;
			this.lhs = lhs;
			this.rhs = rhs;
		}

		public Object evaluate(Tuple tuple) throws JolRuntimeException {
			Number lhs = (Number) this.lhs.evaluate(tuple);
			Number rhs = this.rhs == null ? null : (Number) this.rhs.evaluate(tuple);
			switch(oper) {
			case UPLUS:  return new Short((short) (lhs.shortValue() + (short) 1));
			case UMINUS: return new Short((short) (lhs.shortValue() - (short) 1));
			case PLUS:   return new Short((short) (lhs.shortValue() + rhs.shortValue()));
			case MINUS:  return new Short((short) (lhs.shortValue() - rhs.shortValue()));
			case TIMES:  return new Short((short) (lhs.shortValue() * rhs.shortValue()));
			case DIVIDE: return new Short((short) (lhs.shortValue() / rhs.shortValue()));
			case MOD:    return new Short((short) (lhs.shortValue() % rhs.shortValue()));
			case POW:    return new Short((short)java.lang.Math.pow(lhs.shortValue(),rhs.shortValue()));
			case LSHIFT: return new Short((short)(lhs.shortValue() << rhs.shortValue()));
			case RSHIFT: return new Short((short)(lhs.shortValue() >> rhs.shortValue()));
			default: throw new JolRuntimeException("UNKNOWN MATH OPERATION " + oper);
			}
		}

		public Class returnType() {
			return Short.class;
		}
	}

	public static class SetEval implements TupleFunction {
		private Operator oper;
		private TupleFunction lhs;
		private TupleFunction rhs;

		SetEval(Operator oper, TupleFunction lhs, TupleFunction rhs) {
			this.oper = oper;
			this.lhs = lhs;
			this.rhs = rhs;
		}

		public Object evaluate(Tuple tuple) throws JolRuntimeException {
			Set lhs = (Set) this.lhs.evaluate(tuple);
			Set rhs = (Set) this.rhs.evaluate(tuple);
			Set result = new HashSet();

			switch(oper) {
			case PLUS:
				result.addAll(lhs);
				result.addAll(rhs);
				break;
			case MINUS:
				result.addAll(lhs);
				result.removeAll(rhs);
				break;
			case MOD:
				result.addAll(lhs);
				result.retainAll(rhs);
				break;
			default: throw new JolRuntimeException("UNKNOWN SET OPERATION " + oper);
			}
			return result;
		}

		public Class returnType() {
			return Set.class;
		}
	}

	public static class StringEval implements TupleFunction {
		private Operator oper;
		private TupleFunction lhs;
		private TupleFunction rhs;

		StringEval(Operator oper, TupleFunction lhs, TupleFunction rhs) {
			this.oper = oper;
			this.lhs = lhs;
			this.rhs = rhs;
		}

		public Object evaluate(Tuple tuple) throws JolRuntimeException {
			String lhs = (String) this.lhs.evaluate(tuple);
			String rhs = (String) this.rhs.evaluate(tuple);
			switch (oper) {
			case PLUS: return new String(lhs.toString() + rhs.toString());
			default:   throw new JolRuntimeException("UNKNOWN STRING OPERATION " + oper);
			}
		}

		public Class returnType() {
			return String.class;
		}
	}
}
