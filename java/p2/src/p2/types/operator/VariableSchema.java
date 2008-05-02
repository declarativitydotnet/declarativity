package p2.types.operator;

import java.util.ArrayList;
import java.util.List;

import p2.lang.plan.Variable;
import p2.types.table.Schema;

public class VariableSchema extends Schema {

	public VariableSchema(List<Variable> variables) {
		this.schema = new ArrayList<Entry>();
		for (Variable variable : variables) {
			this.schema.add(new Entry(variable.name(), variable.type()));
		}
	}
	
	public VariableSchema join(VariableSchema input) {
		return null;
	}
	
	public VariableSchema project(VariableSchema input, List<Variable> variables) {
		return null;
	}
	
	public void variable(Variable variable) {
		this.schema.add(new Entry(variable.name(), variable.type()));
	}
	
	public boolean contains(Variable variable) {
		return super.contains(variable.name());
	}
}
