package gfs;

import jol.core.Runtime;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class MasterRequestTable extends ObjectTable {
	public static final TableName TABLENAME = new TableName("gfs_master", "request");
	
	public static final Key PRIMARY_KEY = new Key(0, 1);
	
	public enum Field {
		ID,
		SOURCE,
		CONTENT
	};
	
	public static final Class<?>[] SCHEMA = {
		Integer.class,	// Request ID
		String.class,	// Source address
		String.class	// Request payload
	};

	protected MasterRequestTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
	}
}
