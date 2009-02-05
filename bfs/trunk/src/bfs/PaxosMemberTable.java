package bfs;

import jol.core.Runtime;
import jol.types.table.Key;
import jol.types.table.ObjectTable;
import jol.types.table.TableName;

public class PaxosMemberTable extends ObjectTable {
	public static final TableName TABLENAME = new TableName("paxos_global", "members");

	public static final Key PRIMARY_KEY = new Key(0);

	public enum Field {
        ADDRESS
	};

	public static final Class<?>[] SCHEMA = {
        String.class    // Address of self
	};

	PaxosMemberTable(Runtime context) {
		super(context, TABLENAME, PRIMARY_KEY, SCHEMA);
	}
}
