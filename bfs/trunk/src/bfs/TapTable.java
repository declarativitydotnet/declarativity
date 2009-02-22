package bfs;

import jol.core.Runtime;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class TapTable extends ObjectTable {
	public static final TableName TABLENAME = new TableName("tap", "tap");

	public static final Key PRIMARY_KEY = new Key(0);

	public enum Field {
        SINK,
        PROGRAM
	};

	public static final Class<?>[] SCHEMA = {
        String.class,    // Address of sink
        String.class     // program
	};

	TapTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
	}
}
