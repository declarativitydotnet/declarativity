package jol.types.function;

import jol.lang.plan.Variable;
import jol.types.basic.BasicTupleSet;
import java.util.Set;

public interface SetFunction<Type> {

	public Type evaluate(BasicTupleSet tuples);
	
	public Set<Variable> requires();
}
