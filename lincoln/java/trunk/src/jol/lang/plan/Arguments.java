package jol.lang.plan;

import java.util.ArrayList;
import java.util.List;

public class Arguments extends ArrayList<Expression> {
	private static final long serialVersionUID = 1L;

	private Predicate predicate;
	
	public Arguments(Predicate predicate, List<? extends Expression> arguments) {
		super(arguments.size());
		this.predicate = predicate;
		addAll(arguments);
	}
	
	public Arguments clone() {
		return new Arguments(predicate, this);
	}
	
	@Override
	public String toString() {
		return super.toString();
	}
	
	public boolean equals(Object o) {
		if (o instanceof Arguments) {
			Arguments other = (Arguments) o;
			return this.predicate.equals(other.predicate);
		}
		return false;
	}

}
