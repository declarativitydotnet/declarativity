package jol.lang.plan;

import java.util.ArrayList;
import java.util.List;

import xtc.tree.Node;

import jol.types.basic.Tuple;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;
import jol.types.basic.Schema;;

public class Limit extends Aggregate {
	protected Number kConst;
	
	protected Variable kVar;

	public Limit(Node node, Variable value, Number kConst) {
		this(node, jol.types.function.Aggregate.LIMIT, value, kConst);
	}
	
	public Limit(Node node, Variable value, Variable kVar) {
		this(node, jol.types.function.Aggregate.LIMIT, value, kVar);
	}
	
	protected Limit(Node node, String function, Variable value, Number kConst) {
		super(node, variables(value), function, ValueList.class);
		this.kConst = kConst;
		this.kVar = null;
	}
	
	public Limit(Node node, String function, Variable value, Variable kVar) {
		super(node, variables(value, kVar), function, ValueList.class);
		this.kConst = null;
		this.kVar = kVar;
	}
	
	protected Limit(Limit copy) {
		super(copy.node(), copy.variables, copy.function, ValueList.class);
		this.kConst = copy.kConst;
		this.kVar   = copy.kVar;
	}
	
	@Override
	public Expression clone() {
		return new Limit(this);
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
	public TupleFunction function(Schema schema) throws PlannerException {
		final Number kConst = this.kConst;
		final List<TupleFunction<Comparable>> values = 
			new ArrayList<TupleFunction<Comparable>>();
		for (Variable var : this.variables) {
			values.add(var.function(schema));
		}
		
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				ValueList result = new ValueList();
				for (TupleFunction f : values) {
					result.add(f.evaluate(tuple));
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
