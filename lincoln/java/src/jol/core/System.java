package jol.core;

import java.net.URL;

import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;

/**
 * The interface to the System.
 * User programs interact with the OverLog library
 * via this interface. Such an interface to the system
 * is obtained via the {link Runtime#boostrap(int)} method.
 */
public interface System {
	/**
	 * Get the system clock.
	 * @return The current system (logical) clock
	 */
	Clock clock();
	
	/**
	 * Install program file into runtime.
	 * @param owner Program owner.
	 * @param file File containing program text.
	 * @throws UpdateException compilation/installation problems
	 */
	void install(String owner, URL file) throws UpdateException;

	/**
	 * Uninstall program.
	 * @param program Program name
	 * @throws UpdateException Uninstall problems
	 */
	void uninstall(String program) throws UpdateException;

	/**
	 * Schedule a set of tuples to be evaluated by a given program.
	 * @param program Program name
	 * @param name Tuple name
	 * @param insertions Set of tuples to insert and evaluate
	 * @param deletions Set of tuples to delete and evaluate
	 * @throws UpdateException
	 */
	void schedule(String program,
				  TableName name,
				  TupleSet insertions,
				  TupleSet deletions) throws UpdateException;
}
