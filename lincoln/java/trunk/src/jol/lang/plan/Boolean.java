package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;
import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
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
	public TupleFunction<java.lang.Boolean> function() {
		if (this.oper.equals(AND)) {
			return new TupleFunction<java.lang.Boolean>() {
				private final TupleFunction left  = lhs.function();
				private final TupleFunction right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					return (java.lang.Boolean)left.evaluate(tuple) && (java.lang.Boolean)right.evaluate(tuple);
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(OR)) {
			return new TupleFunction<java.lang.Boolean>() {
				private final TupleFunction left  = lhs.function();
				private final TupleFunction right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					return (java.lang.Boolean)left.evaluate(tuple) || (java.lang.Boolean)right.evaluate(tuple);
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(NOT)) {
			return new TupleFunction<java.lang.Boolean>() {
				private final TupleFunction left  = lhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					return !(java.lang.Boolean)left.evaluate(tuple);
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(EQUAL)) {
			return new TupleFunction<java.lang.Boolean>() {
				private final TupleFunction<C> left  = lhs.function();
				private final TupleFunction<C> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					/* Evaluate only once!! */
					C l = left.evaluate(tuple); 
					C r = right.evaluate(tuple);
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
				private final TupleFunction<C> left  = lhs.function();
				private final TupleFunction<C> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					/* Evaluate only once!! */
					C l = left.evaluate(tuple);
					C r = right.evaluate(tuple);
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
				private final TupleFunction<C> left  = lhs.function();
				private final TupleFunction<C> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					C lvalue = left.evaluate(tuple);
					C rvalue = right.evaluate(tuple);
					try {
						return lvalue.compareTo(rvalue) <= 0;
					} catch (Throwable t) {
						System.err.println("ERROR " + t.toString() + ", ON " + lvalue + " <= " + rvalue);
						throw new JolRuntimeException(t.toString());
					}
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(GEQUAL)) {
			return new TupleFunction<java.lang.Boolean>() {
				private final TupleFunction<C> left  = lhs.function();
				private final TupleFunction<C> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					return (left.evaluate(tuple)).compareTo(right.evaluate(tuple)) >= 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(LESS)) {
			return new TupleFunction<java.lang.Boolean>() {
				private final TupleFunction<C> left  = lhs.function();
				private final TupleFunction<C> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					return (left.evaluate(tuple)).compareTo(right.evaluate(tuple)) < 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(GREATER)) {
			return new TupleFunction<java.lang.Boolean>() {
				private final TupleFunction<C> left  = lhs.function();
				private final TupleFunction<C> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					try {
						Comparable lcomp = left.evaluate(tuple);
						Comparable rcomp = right.evaluate(tuple);
						return lcomp.compareTo(rcomp) > 0;
					} catch (Throwable t) {
						System.err.println("ERROR " + Boolean.this.toString());
						throw new JolRuntimeException(t.toString() + 
								" -- " + Boolean.this.toString() + 
								": left " + left + ", right " + right);
					}
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(IN)) {
			return new TupleFunction<java.lang.Boolean>() {
				private final TupleFunction<C> left  = lhs.function();
				private final TupleFunction<C> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws JolRuntimeException {
					Range.Function<C> range = (Range.Function<C>) right.evaluate(tuple);
					return range.test(left.evaluate(tuple));
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		
		assert(false);
		return null;
	}
}
