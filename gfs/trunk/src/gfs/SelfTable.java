package gfs;

import jol.core.Runtime;
import jol.types.basic.TypeList;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class SelfTable extends ObjectTable {
	public static final TableName TABLENAME = new TableName("gfs_master", "self");
	
	public static final Key PRIMARY_KEY = new Key(0);
	
	public enum Field {
        ADDRESS
	};
	
	public static final Class<?>[] SCHEMA = {
        String.class,   // Address of self
	};

	protected SelfTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, new TypeList(SCHEMA));
	}
}
