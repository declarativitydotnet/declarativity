package jol.core;

import java.net.URL;

import jol.types.basic.BasicTupleSet;
import jol.types.basic.TupleSet;
import jol.types.exception.JolRuntimeException;
import jol.types.exception.UpdateException;
import jol.types.table.TableName;
import jol.types.table.Table.Catalog;

/**
 * The interface to an instance of the JOL runtime. User programs interact with
 * the OverLog library via this interface. Such an interface to the system is
 * obtained via the {@link jol.core.Runtime#create(int)} static method.
 */
public interface JolSystem {

	/**	Get the system lock. */
	Object lock();

	/**
	 * Get the system clock.
	 * @return The current system (logical) clock
	 */
	Clock clock();

	/** Shutdown the system. */
	void shutdown();

	/** Start the asynchronous system driver. */
	void start();

	/** Indicates the last time a call was made to evaluate. */
	Long timestamp();

	/** See {@link Thread#setPriority(int)} */
	void setPriority(int newPriority);

	/**
	 * Perform a single fixpoint computation.
	 * This call will evaluate to fixpoint previous calls to
	 * {@link #schedule(String, TableName, BasicTupleSet, BasicTupleSet)},
	 * {@link #install(String, URL)} and {@link #uninstall(String)}
	 * Calling this method after a call to {@link #start()} is not allowed.
	 * @throws JolRuntimeException When called post {@link #start()}
	 */
	void evaluate() throws JolRuntimeException;

	/**
	 * Get the system catalog.
	 * @return The system catalog {@link Catalog}.
	 */
	Catalog catalog();

	/**
	 * Install program file into runtime.
	 * @param owner Program owner.
	 * @param url The location of the program contents.
	 * @throws UpdateException compilation/installation problems
	 */
	void install(String owner, URL url) throws JolRuntimeException;

	/**
	 * Uninstall program.
	 * @param program Program name
	 * @throws UpdateException Uninstall problems
	 */
	void uninstall(String program) throws JolRuntimeException;

	/**
	 * Schedule a set of tuples to be evaluated by a given program.
	 * The evaluation will occur at a future fixpoint.
	 * @param program Program name
	 * @param name Table name
	 * @param insertions Set of tuples to insert and evaluate
	 * @param deletions Set of tuples to delete and evaluate
	 * @throws UpdateException
	 * @throws JolRuntimeException
	 */
	void schedule(String program,
				  TableName name,
				  TupleSet insertions,
				  TupleSet deletions) throws JolRuntimeException;

	void flusher(TableName name, TupleSet insertions, TupleSet deletions) throws JolRuntimeException;

	/**
	 * Control whether the JOL driver thread runs as a daemon thread or not; by
	 * default, it does not. Can only be invoked before start() has been called.
	 */
	void setDaemon(boolean isDaemon);
}
