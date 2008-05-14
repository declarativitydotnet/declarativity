package p2.types.operator;

import java.util.Set;

import p2.lang.plan.Predicate;
import p2.lang.plan.Variable;
import p2.types.basic.Intermediate;
import p2.types.basic.Schema;

public abstract class Join extends Operator {
	
	protected Predicate predicate;

	public Join(Predicate predicate) {
		super(predicate.program(), predicate.rule(), predicate.position());
		this.predicate = predicate;
	}
	
	@Override
	public Schema schema(Schema input) {
		return input.join(predicate.schema());
	}

	@Override
	public Set<Variable> requires() {
		return predicate.requires();
	}

}
