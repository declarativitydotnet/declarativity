package jol.core;

import java.net.URL;

import jol.types.basic.TupleSet;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;
import jol.types.table.Table.Catalog;

/**
 * The interface to the System.
 * User programs interact with the OverLog library
 * via this interface. Such an interface to the system
 * is obtained via the {@link jol.core.Runtime#create(int)} static method.
 */
public interface System {
	/**
	 * Get the system clock.
	 * @return The current system (logical) clock
	 */
	Clock clock();
	
	/**
	 * Get the system catalog.
	 * @return The system catalog {@link Catalog}.
	 */
	Catalog catalog();

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
	 * @param name Table name
	 * @param insertions Set of tuples to insert and evaluate
	 * @param deletions Set of tuples to delete and evaluate
	 * @throws UpdateException
	 */
	void schedule(String program,
				  TableName name,
				  TupleSet insertions,
				  TupleSet deletions) throws UpdateException;
}
