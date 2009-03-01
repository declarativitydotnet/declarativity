package jol.types.operator;

import jol.core.Runtime;
import jol.lang.plan.Expression;
import jol.types.basic.Tuple;
import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

public class Invoker extends Operator {
	
	private TupleFunction<Object> exprEval;


	public Invoker(Runtime context, TupleFunction exprEval, String program, String rule) {
		super(context, program, rule);
		this.exprEval = exprEval;
	}

	@Override
	public TupleSet evaluate(TupleSet tuples) throws JolRuntimeException {
		for (Tuple t : tuples) {
			this.exprEval.evaluate(t);
		}
		return tuples;
	}

	@Override
	public String toString() {
		return "Invoker operator: rule " + this.rule;
	}

}
