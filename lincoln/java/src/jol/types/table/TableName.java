package jol.types.table;

import java.io.Serializable;

/**
 * Represents the name of a given table.
 * A table name is made up of two parts:
 * 1. The scope of the table (e.g., the program name that created the table).
 * 2. The (user) given name.
 *
 */
public class TableName implements Comparable<TableName>, Serializable {
	
	/** The scope of the table. */
	public String scope;
	
	/** The string name of this table. */
	public String name;
	
	/**
	 * Construct a new table name.
	 * @param scope The table scope (e.g., the program name).
	 * @param name The name of the table.
	 */
	public TableName(String scope, String name) {
		this.scope = scope;
		this.name = name;
	}
	
	@Override
	public String toString() {
		return scope + "::" + name;
	}
	
	@Override
	public boolean equals(Object o) {
		return o instanceof TableName &&
				toString().equals(o.toString());
	}
	
	@Override
	public int hashCode() {
		return toString().hashCode();
	}

	/**
	 * Compare the string version of the table name.
	 */
	public int compareTo(TableName o) {
		return toString().compareTo(o.toString());
	}
	
}

