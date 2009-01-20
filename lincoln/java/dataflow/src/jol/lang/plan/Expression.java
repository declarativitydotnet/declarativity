package jol.lang.plan;

import java.util.Set;

import jol.types.basic.Schema;
import jol.types.exception.PlannerException;
import jol.types.function.TupleFunction;

public abstract class Expression<C> {
	
	private xtc.tree.Location location;
	
	public void location(xtc.tree.Location location) {
		this.location = location;
	}
	
	public xtc.tree.Location location() {
		return this.location;
	}

	@Override
	public abstract String toString();
	
	@Override
	public abstract Expression clone();
	
	/**
	 * @return The java type of the expression value.
	 */
	public abstract Class<C> type();
	
	/**
	 * @return All variables that this expression depends on.
	 */
	public abstract Set<Variable> variables();
	
	/**
	 * Get the function that evaluates this expression. Some
	 * expressions take arguments from input tuples and therefore
	 * need to know the tuple schema from which arguments can be
	 * obtained. 
	 * @param schema Input tuple schema
	 * @return A function that evaluates the expression on an input tuple.
	 * @throws PlannerException Bad input schema.
	 */
	public abstract 
	TupleFunction<C> function(Schema schema) throws PlannerException;
	
}
