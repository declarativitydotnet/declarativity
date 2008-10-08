package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;
import p2.types.basic.Tuple;
import p2.types.exception.P2RuntimeException;
import p2.types.function.TupleFunction;


public class Boolean extends Expression {

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
			return new TupleFunction() {
				private final TupleFunction<java.lang.Boolean> left  = lhs.function();
				private final TupleFunction<java.lang.Boolean> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					return left.evaluate(tuple) && right.evaluate(tuple);
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(OR)) {
			return new TupleFunction() {
				private final TupleFunction<java.lang.Boolean> left  = lhs.function();
				private final TupleFunction<java.lang.Boolean> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					return left.evaluate(tuple) || right.evaluate(tuple);
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(NOT)) {
			return new TupleFunction() {
				private final TupleFunction<java.lang.Boolean> left  = lhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					return !left.evaluate(tuple);
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(EQUAL)) {
			return new TupleFunction() {
				private final TupleFunction<Comparable> left  = lhs.function();
				private final TupleFunction<Comparable> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					Comparable l = left.evaluate(tuple);
					Comparable r = right.evaluate(tuple);
					if (l == null || r == null) return l == r;
					else return left.evaluate(tuple).compareTo(right.evaluate(tuple)) == 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(NEQUAL)) {
			return new TupleFunction() {
				private final TupleFunction<Comparable> left  = lhs.function();
				private final TupleFunction<Comparable> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					Comparable l = left.evaluate(tuple);
					Comparable r = right.evaluate(tuple);
					if (l == null || r == null) return l != r;
					else return left.evaluate(tuple).compareTo(right.evaluate(tuple)) != 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(LEQUAL)) {
			return new TupleFunction() {
				private final TupleFunction<Comparable> left  = lhs.function();
				private final TupleFunction<Comparable> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					return left.evaluate(tuple).compareTo(right.evaluate(tuple)) <= 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(GEQUAL)) {
			return new TupleFunction() {
				private final TupleFunction<Comparable> left  = lhs.function();
				private final TupleFunction<Comparable> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					return left.evaluate(tuple).compareTo(right.evaluate(tuple)) >= 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(LESS)) {
			return new TupleFunction() {
				private final TupleFunction<Comparable> left  = lhs.function();
				private final TupleFunction<Comparable> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					return left.evaluate(tuple).compareTo(right.evaluate(tuple)) < 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(GREATER)) {
			return new TupleFunction() {
				private final TupleFunction<Comparable> left  = lhs.function();
				private final TupleFunction<Comparable> right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					return left.evaluate(tuple).compareTo(right.evaluate(tuple)) > 0;
				}
				public Class returnType() {
					return java.lang.Boolean.class;
				}
				
			};
		}
		else if (this.oper.equals(IN)) {
			return new TupleFunction() {
				private final TupleFunction<Comparable> left  = lhs.function();
				private final Object right = rhs.function();
				public java.lang.Boolean evaluate(Tuple tuple) throws P2RuntimeException {
					if (right instanceof TupleFunction) {
						Range.Function range = (Range.Function) ((TupleFunction)right).evaluate(tuple);
						return range.test(left.evaluate(tuple));
					}
					else {
						Set set = (Set) right;
						return set.contains(left.evaluate(tuple));
					}
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
