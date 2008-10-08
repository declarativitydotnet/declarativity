package jol.types.function;

import jol.lang.plan.Variable;
import jol.types.basic.TupleSet;
import java.util.Set;

public interface SetFunction<Type> {

	public Type evaluate(TupleSet tuples);
	
	public Set<Variable> requires();
}
