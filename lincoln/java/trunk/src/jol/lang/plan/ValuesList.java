package jol.lang.plan;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import jol.types.basic.Tuple;
import jol.types.basic.ValueList;
import jol.types.exception.JolRuntimeException;
import jol.types.function.TupleFunction;

public class ValuesList extends Expression<ValueList> {
	private List<Expression> values;
	
	public ValuesList(List<Expression> values) {
		this.values = values;
	}
	
	public Expression clone() {
		List<Expression> values = new ArrayList<Expression>();
		for (Expression value : this.values) {
			values.add(value.clone());
		}
		return new ValuesList(values);
	}
	
	@Override
	public Class type() {
		return jol.types.basic.ValueList.class;
	}
	
	@Override
	public String toString() {
		return this.values.toString();
	}

	@Override
	public Set<Variable> variables() {
		Set<Variable> variables = new HashSet<Variable>();
		for (Expression value : this.values) {
			variables.addAll(value.variables());
		}
		return variables;
	}

	@Override
	public TupleFunction function() {
		final List<Expression> values = this.values;
		return new TupleFunction() {
			public Object evaluate(Tuple tuple) throws JolRuntimeException {
				ValueList list = new ValueList();
				for (Expression value: values) {
					list.add(value.function().evaluate(tuple));
				}
				return list;
			}
			public Class returnType() {
				return ValueList.class;
			}
		};
	}

}
