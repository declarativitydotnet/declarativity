package jol.test;

import jol.core.Runtime;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class PathTable extends ObjectTable {
	public static final TableName TABLENAME = new TableName("path", "path");
	
	public static final Key PRIMARY_KEY = new Key(0, 1, 2);
	
	public enum Field {
		SOURCE,
		DESTINATION,
		HOPS
	};
	
	public static final Class[] SCHEMA = {
		String.class,	// Source
		String.class,   // Destination
		Integer.class   // # of hops from source => dest on this path
	};
	
	public PathTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
	}
}
