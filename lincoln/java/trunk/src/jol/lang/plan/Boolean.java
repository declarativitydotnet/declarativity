package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;


public class Boolean<C extends Comparable<C>> extends Expression<java.lang.Boolean> {

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

	private String oper;

	private Expression<C> lhs;

	private Expression<C> rhs;

	public Boolean(String oper, Expression<C> lhs, Expression<C> rhs) {
		this.oper = oper;
		this.lhs = lhs;
		this.rhs = rhs;
	}
	
	public Expression clone() {
		return new Boolean(oper, lhs, rhs);
	}

	@Override
	public Class<java.lang.Boolean> type() {
		return java.lang.Boolean.class;
	}

	@Override
	public String toString() {
		return "BOOLEAN(" + lhs.toString() + " " +
		      oper + " " + rhs.toString() + ")";
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
	public TupleFunction<java.lang.Boolean> function(Schema schema) throws PlannerException {
		final TupleFunction<C> lvalue = lhs.function(schema);
		final TupleFunction<C> rvalue = rhs == null ? null : rhs.function(schema);
		
		if (this.oper.equals(AND)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					return (java.lang.Boolean) lvalue.evaluate(tuple) && 
					       (java.lang.Boolean) rvalue.evaluate(tuple);
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		else if (this.oper.equals(OR)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					return (java.lang.Boolean)  lvalue.evaluate(tuple) || 
					       (java.lang.Boolean)  rvalue.evaluate(tuple);
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		else if (this.oper.equals(NOT)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					return !(java.lang.Boolean) lvalue.evaluate(tuple);
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		else if (this.oper.equals(EQUAL)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					/* Evaluate only once!! */
					C l =  lvalue.evaluate(tuple);
					C r =  rvalue.evaluate(tuple);
					if (l == null || r == null) return l == r;
					else return l.compareTo(r) == 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		else if (this.oper.equals(NEQUAL)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					/* Evaluate only once!! */
					C l =  lvalue.evaluate(tuple);
					C r =  rvalue.evaluate(tuple);
					if (l == null || r == null) return l != r;
					else return l.compareTo(r) != 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		else if (this.oper.equals(LEQUAL)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					C left  = lvalue.evaluate(tuple);
					C right = rvalue.evaluate(tuple);
					try {
						return left.compareTo(right) <= 0;
					} catch (Throwable t) {
						String msg = "ERROR " + t.toString() + ", ON " + lvalue + " <= " + rvalue;
						throw new JolRuntimeException(msg, t);
					}
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		else if (this.oper.equals(GEQUAL)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
                    try {
                        C left  = lvalue.evaluate(tuple);
                        C right = rvalue.evaluate(tuple);
                        return left.compareTo(right) >= 0;
                    } catch (Throwable t) {
                        throw new JolRuntimeException(t.toString() +
                                " -- " + Boolean.this.toString() +
                                ":  lvalue " +  lvalue + ",  rvalue " +  rvalue, t);
                    }
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		else if (this.oper.equals(LESS)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
				    try {
				        C left  = lvalue.evaluate(tuple);
				        C right = rvalue.evaluate(tuple);
				        return left.compareTo(right) < 0;
				    } catch (Throwable t) {
                        throw new JolRuntimeException(t.toString() +
                                " -- " + Boolean.this.toString() +
                                ":  lvalue " +  lvalue + ",  rvalue " +  rvalue, t);
				    }
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		else if (this.oper.equals(GREATER)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					try {
						C left  = lvalue.evaluate(tuple);
						C right = rvalue.evaluate(tuple);
						return left.compareTo(right) > 0;
					} catch (Throwable t) {
						throw new JolRuntimeException(t.toString() +
								" -- " + Boolean.this.toString() +
								":  lvalue " +  lvalue + ",  rvalue " +  rvalue, t);
					}
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		else if (this.oper.equals(IN)) {
			return new TupleFunction<java.lang.Boolean>() {
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					Range.Function<C> range = (Range.Function<C>) rvalue.evaluate(tuple);
					return range.test(lvalue.evaluate(tuple));
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
			};
		}
		throw new PlannerException("Unknown operator " + this.oper);
	}
}
