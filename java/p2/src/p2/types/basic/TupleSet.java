package p2.types.basic;

import java.io.Serializable;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import p2.types.table.TableName;


public class TupleSet extends HashSet<Tuple> implements Comparable<TupleSet>, Serializable {
	private static long ids = 0L;
	
	private String id;
	
	private TableName name;
	
	public TupleSet() {
		this((TableName)null);
	}
	
	public TupleSet(TableName name) {
		this.id = "TupleSet:" + ids++;
		this.name = name;
	}
	
	public TupleSet(TableName name, Set<Tuple> tuples) {
		this.id = "TupleSet:" + ids++;
		this.name = name;
		this.addAll(tuples);
	}
	
	public TupleSet(TableName name, Tuple tuple) {
		this.id = "TupleSet:" + ids++;
		this.name = name;
		this.add(tuple);
	}
	
	public String toString() {
		String tuples = name + "[\n";
		for (Tuple tuple : this) {
			tuples += "\t" + tuple + "\n";
		}
		tuples += "]\n";
		return tuples;
	}
	
	@Override
	public int hashCode() {
		return id.hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof TupleSet) {
			return ((TupleSet)o).id.equals(this.id);
		}
		return false;
	}
	
	public TupleSet clone() {
		return new TupleSet(name, this);
	}
	
	public TableName name() {
		return this.name;
	}

	/**
	 * The only meaningful response to this
	 * method is to determine if the two sets
	 * are equal.
	 */
	public int compareTo(TupleSet tuples) {
		return this.id.compareTo(tuples.id);
	}
	
	
}
	
