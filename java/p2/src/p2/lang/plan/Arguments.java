package p2.lang.plan;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import p2.types.function.TupleFunction;

public class Arguments extends ArrayList<Expression> implements Comparable<Arguments> {

	private Predicate predicate;
	
	public Arguments(Predicate predicate, List<Expression> arguments) {
		super(arguments.size());
		this.predicate = predicate;
		addAll(arguments);
	}
	
	@Override
	public String toString() {
		return super.toString();
	}
	
	public int compareTo(Arguments a) {
		return this.predicate.compareTo(a.predicate);
	}

}
