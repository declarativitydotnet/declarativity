package p2.core;

import java.net.URL;

import p2.types.basic.TupleSet;
import p2.types.exception.UpdateException;
import p2.types.table.TableName;

public interface System {
	/**
	 * Initialize this instance of the P2 runtime.
	 * 
	 * @param port The network port to use
	 */
	void bootstrap(int port);

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