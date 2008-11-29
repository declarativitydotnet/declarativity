package jol.lang.plan;

import java.util.ArrayList;
import java.util.List;

import jol.types.basic.Tuple;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

public class Limit extends Aggregate {
	private Number kConst;
	
	private Variable kVar;

	public Limit(Variable value, Number kConst) {
		this(jol.types.function.Aggregate.LIMIT, value, kConst);
	}
	
	public Limit(Variable value, Variable kVar) {
		this(jol.types.function.Aggregate.LIMIT, value, kVar);
	}
	
	protected Limit(String function, Variable value, Number kConst) {
		super(variables(value), function, ValueList.class);
		this.kConst = kConst;
		this.kVar = null;
	}
	
	public Limit(String function, Variable value, Variable kVar) {
		super(variables(value, kVar), function, ValueList.class);
		this.kConst = null;
		this.kVar = kVar;
	}
	
	private static List<AggregateVariable> variables(Variable...variables) {
		List<AggregateVariable> aggs = new ArrayList<AggregateVariable>();
		for (Variable var : variables) {
			aggs.add(new AggregateVariable(var));
		}
		return aggs;
	}
	
	public Number kConst() {
		return this.kConst;
	}
	
	public Variable kVar() {
		return this.kVar;
	}
	
	@Override
	public TupleFunction function() {
		final List<AggregateVariable> variables = this.variables;
		final Number kConst = this.kConst;
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				ValueList result = new ValueList();
				for (AggregateVariable var : variables) {
					result.add((Comparable)var.function().evaluate(tuple));
				}
				if (variables.size() == 1) {
					result.add(kConst);
				}
				return result;
			}

			public Class returnType() {
				return ValueList.class;
			}
		};
	}
}
