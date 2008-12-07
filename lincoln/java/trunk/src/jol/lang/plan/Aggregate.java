package jol.lang.plan;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.types.basic.Schema;
import jol.types.basic.Tuple;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;


public class Aggregate extends Expression {
	protected Class type;
	
	protected List<AggregateVariable> variables;
	
	protected String function;
	
	protected MethodCall method;
	
	public Aggregate(AggregateVariable variable, String function, Class type) {
		this.variables = new ArrayList<AggregateVariable>();
		this.variables.add(variable);
		this.function = function;
		this.type = type;
		this.method = null;
	}
	
	public Aggregate(List<AggregateVariable> variables, String function, Class type) {
		this.type = type;
		this.variables = variables;
		this.function = function;
		this.method = null;
	}
	
	public Aggregate(MethodCall method, String function, Class type) {
		this(formAggregateVariables(method.variables()), function, type);
		this.method = method;
	}
	
	private static List<AggregateVariable> formAggregateVariables(Set<Variable> variables) {
		List<AggregateVariable> aggregateVariables = new ArrayList<AggregateVariable>();
		for (Variable var : variables) {
			aggregateVariables.add(new AggregateVariable(var));
		}
		return aggregateVariables;
	}
	
	@Override
	public Expression clone() {
		return this.method == null ? new Aggregate(this.variables, this.function, type()) :
									 new Aggregate(method, function, type());
	}

	@Override
	public String toString() {
		String agg = this.function + "<";
		for (Variable var : this.variables) {
			agg += var.name() + ", ";
		}
		agg = agg.substring(0, agg.lastIndexOf(", ")) + ">";
		return agg;
	}
	
	public String functionName() {
		return this.function;
	}
	
	@Override
	public TupleFunction function(Schema schema) throws PlannerException {
		if (this.method != null) {
			return this.method.function(schema);
		}
		else if (this.variables.size() > 0) {
			final List<TupleFunction<Comparable>> variableFunctions = 
				new ArrayList<TupleFunction<Comparable>>();
			for (Expression var : this.variables) {
				variableFunctions.add(var.function(schema));
			}
			return new TupleFunction() {
				public Object evaluate(Tuple tuple) throws JolRuntimeException {
					if (variables.size() == 1) {
						return variableFunctions.get(0).evaluate(tuple);
					} else {
						ValueList result = new ValueList();
						for (TupleFunction<Comparable> fn : variableFunctions) {
							result.add(fn.evaluate(tuple));
						}
						return result;
					}
				}

				public Class returnType() {
					return variables.size() == 1 ? 
							variables.get(0).type() : ValueList.class;
				}
			};
		}
		else {
			throw new PlannerException("Unexpected aggregation case! " + toString());
		}
	}

	@Override
	public Class type() {
		return this.type;
	}

	@Override
	public Set<AggregateVariable> variables() {
		return new HashSet<AggregateVariable>(this.variables);
	}
}
