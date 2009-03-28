package jol.types.table;

import java.io.Serializable;

/**
 * Represents the name of a given table.
 * A table name is made up of two parts:
 * 1. The scope of the table (e.g., the program name that created the table).
 * 2. The (user) given name.
 */
public class TableName implements Comparable<TableName>, Serializable {
	private static final long serialVersionUID = 1L;

	/** The scope of the table. */
	public String scope;

	/** The string name of this table. */
	public final String name;

	/**
	 * Construct a new table name.
	 * @param scope The table scope (e.g., the program name).
	 * @param name The name of the table.
	 */
	public TableName(String scope, String name) {
		this.scope = scope;
		this.name = name;
	}

	/**
	 * Construct a table name from a {@link java.lang.String}.
	 * The name format should be scope::name, where scope is
	 * the program defining the table. If no scope is provided
	 * then the scope will default to the global namespace (i.e., global::name).
	 * @param name The name
	 */
	public TableName(String name) {
		if (name.contains("::")) {
			this.scope = name.substring(0, name.indexOf("::"));
			this.name  = name.substring(name.indexOf("::") + "::".length());
		}
		else {
			this.scope = Table.GLOBALSCOPE;
			this.name  = name;
		}
	}

	public String dotLabel() {
		return scope + name;
	}

	public String toDot() {
		String dot = "\n node [label = \"";
		dot += toString() + "\"] " + dotLabel() + ";\n";
		return dot;
	}

	@Override
	public String toString() {
		return scope + "::" + name;
	}

	@Override
	public boolean equals(Object o) {
	    if (!(o instanceof TableName))
	        return false;

	    TableName other = (TableName) o;
	    if (!other.name.equals(this.name))
	        return false;
	    if (!other.scope.equals(this.scope))
	        return false;

	    return true;
	}

	@Override
	public int hashCode() {
	    return this.scope.hashCode() ^ this.name.hashCode();
	}

	/**
	 * Compare the scope, then the string version of the table name.
	 */
	public int compareTo(TableName o) {
	    int scopeCmp = this.scope.compareTo(o.scope);
	    if (scopeCmp != 0)
	        return scopeCmp;

	    return this.name.compareTo(o.name);
	}
}

