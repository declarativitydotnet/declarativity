package p2.types.operator;

import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;

public abstract class Join extends Operator {
	
	private Predicate predicate;

	public Join(String ID, Predicate predicate) {
		super(ID);
		this.predicate = predicate;
	}

	@Override
	public Set<Variable> requires() {
		return predicate.requires();
	}

	@Override
	public VariableSchema schema(VariableSchema input) {
		if (input == null) {
			return this.predicate.schema();
		}
		return this.predicate.schema().join(input);
	}

}
