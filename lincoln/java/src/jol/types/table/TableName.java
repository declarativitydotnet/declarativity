package jol.types.table;

import java.io.Serializable;

public class TableName implements Comparable<TableName>, Serializable {
	
	public String scope;
	public String name;
	
	public TableName(String scope, String name) {
		this.scope = scope;
		this.name = name;
	}
	
	public String toString() {
		return scope + "::" + name;
	}
	
	public boolean equals(Object o) {
		return o instanceof TableName &&
				toString().equals(o.toString());
	}
	
	public int hashCode() {
		return toString().hashCode();
	}

	public int compareTo(TableName o) {
		return toString().compareTo(o.toString());
	}
	
}

