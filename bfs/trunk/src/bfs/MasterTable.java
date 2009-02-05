package bfs;

import jol.core.Runtime;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class MasterTable extends ObjectTable {
	public static final TableName TABLENAME = new TableName("bfs", "master_for_node");

	public static final Key PRIMARY_KEY = new Key(0);

	public enum Field {
        ADDRESS,
        MASTER
	};

	public static final Class<?>[] SCHEMA = {
        String.class,    // Address of self
        String.class     // Address of current master
	};

	MasterTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
	}
}
