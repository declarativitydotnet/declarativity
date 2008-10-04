package p2.core;

import p2.types.basic.TupleSet;
import p2.types.exception.UpdateException;
import p2.types.table.TableName;

public interface System {
	
	/**
	 * Get the system clock.
	 * @return The current system (logical) clock
	 */
	public Clock clock();
	
	/**
	 * Install program file into runtime.
	 * @param owner Program owner.
	 * @param file File containing program text.
	 * @throws UpdateException compilation/installation problems
	 */
	public void install(String owner, 
						String file) throws UpdateException;

	/**
	 * Uninstall program.
	 * @param program Program name
	 * @throws UpdateException Uninstall problems
	 */
	public void uninstall(String program) throws UpdateException;

	/**
	 * Schedule a set of tuples to be evaluated by a given program.
	 * @param program Program name
	 * @param name Tuple name
	 * @param insertions Set of tuples to insert and evaluate
	 * @param deletions Set of tuples to delete and evaluate
	 * @throws UpdateException
	 */
	public void schedule(String program, 
						 TableName name, 
                         TupleSet insertions, 
                         TupleSet deletions) throws UpdateException;
}