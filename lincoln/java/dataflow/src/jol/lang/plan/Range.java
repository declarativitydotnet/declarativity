package jol.lang.plan;

import java.util.HashSet;
import java.util.Set;

import xtc.tree.Node;

import jol.types.basic.Tuple;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;
import jol.types.basic.Schema;

public class Range<C extends Comparable<C>> extends Expression {
	
	public interface Function<C extends Comparable<C>>{
		public boolean test(C test);
	}
	
	private class FunctionCC implements Function<C> {
		private C begin;
		private C end;
		public FunctionCC(C begin, C end) {
			this.begin = begin;
			this.end = end;
		}
		public boolean test(C test) {
			return begin.compareTo(test) <= 0 &&
				   test.compareTo(end) <= 0;
		}
	}
	
	private class FunctionOC implements Function<C> {
		private C begin;
		private C end;
		public FunctionOC(C begin, C end) {
			this.begin = begin;
			this.end = end;
		}
		public boolean test(C test) {
			return begin.compareTo(test) < 0 &&
				   test.compareTo(end) <= 0;
		}
	}
	
	private class FunctionCO implements Function<C> {
		private C begin;
		private C end;
		public FunctionCO(C begin, C end) {
			this.begin = begin;
			this.end = end;
		}
		public boolean test(C test) {
			return begin.compareTo(test) <= 0 &&
				   test.compareTo(end) < 0;
		}
	}
	
	private class FunctionOO implements Function<C> {
		private C begin;
		private C end;
		public FunctionOO(C begin, C end) {
			this.begin = begin;
			this.end = end;
		}
		public boolean test(C test) {
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
	
	private Expression<C> begin;
	
	private Expression<C> end;

	public Range(Node node, Operator oper, Expression<C> begin, Expression<C> end) {
		super(node);
		this.oper = oper;
		this.begin = begin;
		this.end = end;
	}
	
	public Expression clone() {
		return new Range(node(), oper, begin.clone(), end.clone());
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
	public TupleFunction function(Schema schema) throws PlannerException {
		final TupleFunction<C> startFn = begin.function(schema);
		final TupleFunction<C> endFn   = end.function(schema);
		return new TupleFunction() {
			public Function evaluate(Tuple tuple) throws JolRuntimeException {
				C start = (C)startFn.evaluate(tuple);
				C end   = (C)endFn.evaluate(tuple);
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
