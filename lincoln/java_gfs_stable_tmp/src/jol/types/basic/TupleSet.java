package jol.types.basic;

import java.io.Serializable;
import java.util.Collection;
import java.util.Iterator;
import java.util.HashSet;
import java.util.Set;

import jol.types.table.TableName;


/**
 * A tuple set is a set contain for tuples that belong to the same relation.
 */
public class TupleSet extends HashSet<Tuple> implements Comparable<TupleSet>, Serializable {
	private static final long serialVersionUID = 1L;

	/** Identifier generator. */
	private static long ids = 0L;
	
	/** Tuple set identifier. */
	private String id;
	
	/** Table name to which the tuples of this set belong. */
	private TableName name;

	private boolean warnedAboutBigTable = false;
	
	/**
	 * Create an empty tuple set.
	 */
	public TupleSet() {
		this((TableName) null);
	}
	
	/** 
	 * Copy constructor.
	 * @param clone The set to copy.
	 */
	private TupleSet(TupleSet clone) {
		this.id = clone.id;
		this.name = clone.name;
		this.addAll(clone);
	}
	
	/**
	 * Empty tuple set that references a given table name.
	 * @param name The table name to reference.
	 */
	public TupleSet(TableName name) {
		this.id = "TupleSet:" + ids++;
		this.name = name;
	}
	
	/**
	 * Initialize the tuple set to contain the passed in tuples
	 * that reference the given table name.
	 * @param name The table name.
	 * @param tuples The tuples to initialize.
	 */
	public TupleSet(TableName name, Set<Tuple> tuples) {
		this.id = "TupleSet:" + ids++;
		this.name = name;
		this.addAll(tuples);
	}
	
	/**
	 * Create a tuple set referencing the given table name containing
	 * the single tuple.
	 * @param name The table name.
	 * @param tuple A single tuple that will make up this set.
	 */
	public TupleSet(TableName name, Tuple tuple) {
		this.id = "TupleSet:" + ids++;
		this.name = name;
		this.add(tuple);
	}
	
	@Override
	public String toString() {
		String tuples = name + "[";
		Iterator<Tuple> iter = this.iterator();
		while (iter.hasNext()) {
			tuples += iter.next();
			if (iter.hasNext())
				tuples += ", ";
		}
		tuples += "]";
		return tuples;
	}
	
	@Override
	public int hashCode() {
		return id.hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof TupleSet) {
			return ((TupleSet) o).id.equals(this.id);
		}
		return false;
	}
	
	@Override
	public TupleSet clone() {
		return new TupleSet(this);
	}
	
	/**
	 * The tuple set identifier.
	 * @return The identifier assigned to this tuple set.
	 */
	public String id() {
		return this.id;
	}
	
	/**
	 * The name of the table that the tuples belonging to this set are
	 * part of.
	 * @return The table name.
	 */
	public TableName name() {
		return this.name;
	}
	
	/**
	 * Set the table name.
	 * @param name the table name.
	 */
	public void name(TableName name) {
		this.name = name;
	}
	
	public boolean addAll(Iterable<? extends Tuple> tuples) {
		for (Tuple t : tuples)
			add(t);

		if (this.size() > 1000 && !this.warnedAboutBigTable) {
			this.warnedAboutBigTable = true;
			System.err.println("TUPLE SET " + name() + " has exceeded 1000 tuples");
		}

		return true;
	}

	/**
	 * Comparison for tuple identifiers.
	 * 
	 * NOTE: This method does NOT perform set comparison 
	 * (other methods will perform that action {@link #containsAll(Collection)}).
	 */
	public int compareTo(TupleSet tuples) {
		return this.id.compareTo(tuples.id);
	}
}
	
