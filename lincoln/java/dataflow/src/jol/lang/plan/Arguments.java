package jol.lang.plan;

import java.util.ArrayList;
import java.util.List;

public class Arguments extends ArrayList<Expression> implements Comparable<Arguments> {
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
	
	public int compareTo(Arguments a) {
		return this.predicate.compareTo(a.predicate);
	}

}
