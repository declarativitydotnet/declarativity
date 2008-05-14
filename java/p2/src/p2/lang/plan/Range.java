package p2.lang.plan;

import java.util.HashSet;
import java.util.Set;

import p2.types.basic.Tuple;
import p2.types.exception.RuntimeException;
import p2.types.function.TupleFunction;

public class Range extends Expression {
	
	public interface Function {
		public boolean test(Comparable test);
	}
	
	private class FunctionCC implements Function {
		private Comparable begin;
		private Comparable end;
		public FunctionCC(Comparable begin, Comparable end) {
			this.begin = begin;
			this.end = end;
		}
		public boolean test(Comparable test) {
			return begin.compareTo(test) <= 0 &&
				   test.compareTo(end) <= 0;
		}

	}
	
	private class FunctionOC implements Function {
		private Comparable begin;
		private Comparable end;
		public FunctionOC(Comparable begin, Comparable end) {
			this.begin = begin;
			this.end = end;
		}
		public boolean test(Comparable test) {
			return begin.compareTo(test) < 0 &&
				   test.compareTo(end) <= 0;
		}
	}
	
	private class FunctionCO implements Function {
		private Comparable begin;
		private Comparable end;
		public FunctionCO(Comparable begin, Comparable end) {
			this.begin = begin;
			this.end = end;
		}
		public boolean test(Comparable test) {
			return begin.compareTo(test) <= 0 &&
				   test.compareTo(end) < 0;
		}
	}
	
	private class FunctionOO implements Function {
		private Comparable begin;
		private Comparable end;
		public FunctionOO(Comparable begin, Comparable end) {
			this.begin = begin;
			this.end = end;
		}
		public boolean test(Comparable test) {
			return begin.compareTo(test) < 0 &&
				   test.compareTo(end) < 0;
		}
	}
	
	
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

	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		variables.addAll(this.begin.variables());
		variables.addAll(this.end.variables());
		return variables;
	}

	@Override
	public TupleFunction function() {
		return new TupleFunction() {
			private final TupleFunction startFn = begin.function();
			private final TupleFunction endFn   = end.function();

			public Function evaluate(Tuple tuple) throws RuntimeException {
				Comparable start = (Comparable)startFn.evaluate(tuple);
				Comparable end   = (Comparable)endFn.evaluate(tuple);
				if (oper == Operator.CC)
					return new FunctionCC(start, end);
				if (oper == Operator.OC)
					return new FunctionOC(start, end);
				if (oper == Operator.CO)
					return new FunctionCO(start, end);
				if (oper == Operator.OO)
					return new FunctionOO(start, end);
				assert(false);
				return null;
			}

			public Class returnType() {
				return Function.class;
			}
			
		};

	}

}
