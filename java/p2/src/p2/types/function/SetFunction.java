package p2.types.function;

import p2.lang.plan.Variable;
import p2.types.basic.TupleSet;
import java.util.Set;

public interface SetFunction<Type> {

	public Type evaluate(TupleSet tuples);
	
	public Set<Variable> requires();
}
