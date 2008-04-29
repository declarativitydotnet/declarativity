package p2.core;

import p2.lang.Compile;
import p2.types.exception.UpdateException;
import p2.types.table.Index;
import p2.types.table.Table;

public class System {
	

	public static void initialize() {
		Table.initialize();
		Compile.initialize();
	}
}
